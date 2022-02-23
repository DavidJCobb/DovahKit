#include "load_order_persistent_ref_reparenter.h"
#include "file_loader.h"
#include "../file_load_order.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_stub_helpers.h"
#include "../../../helpers/vector3.h"

#include "../../forms/Cell.h"

namespace {
   // Skyrim.esm places 15,196 references into [CELL:00000D74], the persistent cell for [WRLD:0000003C]Tamriel.
   static constexpr int count_to_prepare_for = 15196;

   // For each worldspace, gather relevant child cells into a std::vector before working, for faster searches.
   static constexpr bool pre_list_world_cells = true;

   struct dummy {
      dummy() {}
      template<typename T> dummy(T&) {};
   };
}

namespace dovah::tes_file_reading {
   #pragma region load_order_persistent_ref_reparenter::worker
   load_order_persistent_ref_reparenter::worker::worker() {
      this->queue.reserve(count_to_prepare_for / thread_count + 1);
   }

   void load_order_persistent_ref_reparenter::worker::_execute() {
      using _cell_list = std::conditional_t<pre_list_world_cells, cached_cell_list&, dummy>;
      if constexpr (pre_list_world_cells) {
         assert(this->cells != nullptr);
      }
      _cell_list my_cells = *this->cells;
      //
      auto& list = this->queue;
      this->progress.maximum = list.size();
      for (auto* stub : list) {
         stub->_do_custom_parse(this, [this, &my_cells](form_stub& stub, record& record, dovah::load_order_interfaces::form_load& intfc) {
            if (!intfc.is_winning_record)
               return;
            float x = 0.0;
            float y = 0.0;
            while (auto& subrecord = record.next_subrecord()) {
               if (subrecord.signature() != 'DATA')
                  continue;
               if (!subrecord.is_in_bounds(8))
                  break;
               subrecord.unchecked_read(x);
               subrecord.unchecked_read(y);
               break;
            }
            //
            static_assert(loaded_forms::Cell::side_length == 4096, "If the cell side length isn't 4096, then change these bitshifts into divisions by the side length.");
            auto gx = (int32_t)x >> 0xC;
            auto gy = (int32_t)y >> 0xC;
            //
            form_stub* cell;
            if constexpr (pre_list_world_cells) {
               cell = nullptr;
               for (auto* stub : my_cells) {
                  auto& sg = stub->addenda->grid_coords;
                  if (sg.x == gx && sg.y == gy) {
                     cell = stub;
                     break;
                  }
               }
            } else {
               cell = form_stub_helpers::get_worldspace_cell_by_grid(this->world, gx, gy); // these shifts are division by 4096
            }
            //
            if (cell)
               stub._set_parent_form_one_way(cell);
         });
         ++this->progress.current;
      }
      this->world = nullptr;
   }
   //
   void load_order_persistent_ref_reparenter::worker::set_world(form_stub& world) noexcept {
      this->world = &world;
      this->queue.clear();
      this->queue.reserve(count_to_prepare_for / thread_count + 1);
      this->progress.current = 0;
      this->progress.maximum = 0;
   }
   void load_order_persistent_ref_reparenter::worker::set_cached_cell_list(cached_cell_list& list) {
      this->cells = &list;
   }
   void load_order_persistent_ref_reparenter::worker::add_to_queue(form_stub& stub) noexcept {
      this->queue.push_back(&stub);
   }
   void load_order_persistent_ref_reparenter::worker::start() noexcept {
      assert(this->world);
      assert(!this->is_active());
      this->thread = std::thread(load_order_persistent_ref_reparenter::worker::_thread_handler, this);
   }
   void load_order_persistent_ref_reparenter::worker::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
   float load_order_persistent_ref_reparenter::worker::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
   #pragma endregion

   #pragma region load_order_persistent_ref_reparenter
   load_order_persistent_ref_reparenter::load_order_persistent_ref_reparenter() {
      this->positions.reserve(count_to_prepare_for);
   }

   void load_order_persistent_ref_reparenter::_receive(form_stub* stub, coordinates_t position) {
      auto guard = std::lock_guard(this->lock);
      this->positions[stub] = position;
   }
   void load_order_persistent_ref_reparenter::execute(file_load_order& flo) {
      if constexpr (pre_list_world_cells) {
         for (auto& thread : this->threads)
            thread.set_cached_cell_list(this->cells_for_current_world);
      }
      flo.for_each_form_of_type(dovah::form_type::worldspace, [this](dovah::form_stub* world) -> bool {
         auto* addenda = world->addenda;
         if (!addenda)
            return false;
         auto* p_cell = addenda->persistent_cell;
         if (!p_cell)
            return false;
         //
         if constexpr (pre_list_world_cells) {
            this->cells_for_current_world.clear();
            for (auto& pair : world->inbound) {
               auto& entry = pair.second;
               if (!(entry.flags & use_info_entry::flag::parent_child))
                  continue;
               auto* cell = entry.other;
               if (!cell || cell->formType != form_type::cell)
                  continue;
               assert(cell->get_parent_form() == world && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
               if (cell == p_cell)
                  continue;
               if (!cell->addenda)
                  continue;
               this->cells_for_current_world.push_back(cell);
            }
         }
         int index = 0;
         for (auto& thread : this->threads) {
            thread.set_world(*world); // also resets thread state
         }
         //
         // We need to bidirectionally sever the references from the persistent cell children 
         // to the persistent cell, and then bidirectionally create references from the 
         // persistent cell children to their true parent cells. However, making these changes 
         // as a single operation is impossible to multi-thread and, in general, very slow. 
         // Instead, let's break those operations apart.
         //
         // We'll start by both queuing multi-threaded changes to the cell children, and 
         // one-way-severing the inbound references to the persistent cell.
         //
         {
            use_info_list entries_to_keep;
            for (auto& pair : p_cell->inbound) {
               auto& entry = pair.second;
               if (entry.flags & use_info_entry::flag::parent_child) {
                  auto* child = entry.other;
                  if (child && form_type_info::form_type_is_reference(child->formType)) {
                     this->threads[index].add_to_queue(*child);
                     index = (index + 1) % this->threads.size();
                     //
                     if (--entry.refcount == 0)
                        continue;
                  }
               }
               entries_to_keep[pair.first] = entry;
            }
            std::swap(p_cell->inbound, entries_to_keep);
         }
         //
         // So now, the cell children still refer to the persistent cell, but the persistent 
         // cell is no longer aware of this.
         //
         for (auto& thread : this->threads)
            thread.start();
         for (auto& thread : this->threads)
            thread.wait_for();
         //
         // The multi-threaded process will have both identified the proper parent cell to 
         // move the child to, and performed a one-way set operation to change the child's 
         // parent.
         // 
         // Now, we must make those references bidirectional. Unfortunately, this is the 
         // step of the process that cannot be multi-threaded.
         //
         for (auto& thread : this->threads) {
            auto& queue = thread.view_queue();
            for (auto* stub : queue) {
               auto* cell = stub->get_parent_form();
               assert(cell);
               cell->receive_inbound_ref(stub, 1, use_info_entry::flag::parent_child);
            }
         }
         return false;
      });
   }
   #pragma endregion
}