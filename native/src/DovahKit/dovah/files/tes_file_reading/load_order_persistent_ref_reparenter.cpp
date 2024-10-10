#include "load_order_persistent_ref_reparenter.h"
#include "file_loader.h"
#include "../file_load_order.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../utils/world_position_to_grid_coordinates.h"
#include "../../load_order_interfaces/form_load.h"

#include "../../forms/Cell.h"

namespace {
   // Skyrim.esm places 15,196 references into [CELL:00000D74], the persistent cell for [WRLD:0000003C]Tamriel.
   static constexpr int count_to_prepare_for = 15196;
}

namespace dovah::tes_file_reading {
   #pragma region load_order_persistent_ref_reparenter::world_cell_map
      form_stub* load_order_persistent_ref_reparenter::world_cell_map::cell_for_position(float x, float y) const {
         auto [gx, gy] = world_position_to_grid_coordinates(x, y);
         if (auto* item = this->sorted.at(gx, gy)) {
            return *item;
         }
         for (auto* cell : this->unsorted) {
            const auto& sg = cell->addenda->grid_position;
            if (!sg.has_value())
               continue;
            if (sg.value().x == gx && sg.value().y == gy)
               return cell;
         }
         return nullptr;
      }
      void load_order_persistent_ref_reparenter::world_cell_map::set_world(form_stub& world) {
         this->world = &world;
         auto* p_cell = world.addenda ? world.addenda->persistent_cell : nullptr;
         for (auto& pair : world.inbound) {
            auto& entry = pair.second;
            if (!(entry.flags & use_info_entry::flag::parent_child))
               continue;
            auto* cell = entry.other;
            if (!cell || cell->form_type != form_type::cell)
               continue;
            assert((cell->get_parent_form() == &world) && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
            if (cell == p_cell)
               continue;
            if (!cell->addenda || !cell->addenda->grid_position.has_value())
               continue;
            //
            auto gx = cell->addenda->grid_position.value().x;
            auto gy = cell->addenda->grid_position.value().y;
            if (auto* item = this->sorted.at(gx, gy))
               *item = cell;
            else
               this->unsorted.push_back(cell);
         }
      }

      void load_order_persistent_ref_reparenter::world_cell_map::clear() {
         this->world = nullptr;
         this->unsorted.clear();
         this->sorted.clear();
      }
   #pragma endregion

   #pragma region load_order_persistent_ref_reparenter::worker
   load_order_persistent_ref_reparenter::worker::worker(load_order_persistent_ref_reparenter& o) : owner(o) {
   }

   void load_order_persistent_ref_reparenter::worker::_execute() {
      auto&       list  = this->owner.refs;
      const auto& cells = this->owner.cells_for_current_world;
      for (size_t i = this->range.start; i < this->range.end; ++i) {
         auto& item = list[i];
         item.refr->do_custom_parse_during_serialization({}, this, [&item, &cells](form_stub& refr, record& record, dovah::load_order_interfaces::form_load& intfc) {
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
            form_stub* cell = cells.cell_for_position(x, y);
            if (cell) {
               assert(cell->addenda->grid_position.has_value());
               refr._set_parent_form_one_way({}, cell);
               item.cell = cell;
               item.gx   = cell->addenda->grid_position.value().x;
               item.gy   = cell->addenda->grid_position.value().y;
            }
         });
         ++this->progress.current;
      }
   }
   //
   void load_order_persistent_ref_reparenter::worker::set_range(size_t start, size_t end) {
      this->range.start = start;
      this->range.end   = end;
   }
   void load_order_persistent_ref_reparenter::worker::start() noexcept {
      assert(!this->is_active());
      this->progress.current = 0;
      this->progress.maximum = this->range.end - this->range.start;
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

   #pragma region ...::bidi_worker
   load_order_persistent_ref_reparenter::bidi_worker::bidi_worker(load_order_persistent_ref_reparenter& o, size_t qi, bool xp, bool yp) : owner(o), quartet_index(qi), x_pos(xp), y_pos(yp) {
   }

   void load_order_persistent_ref_reparenter::bidi_worker::_execute() {
      constexpr bool use_sign_masking = false;
      constexpr auto sign_bit_mask    = std::bit_cast<int32_t, uint32_t>(uint32_t(1 << 31));
      const uint32_t x_mask = this->x_pos ? 0 : sign_bit_mask;
      const uint32_t y_mask = this->y_pos ? 0 : sign_bit_mask;
      //
      const auto& list = this->owner.refs;
      size_t start;
      size_t end;
      if constexpr (bidi_worker_quartet_count == 1) {
         start = 0;
         end   = list.size();
      } else {
         const size_t refs_per_quartet = list.size() / bidi_worker_quartet_count;
         start = this->quartet_index * refs_per_quartet;
         end   = (this->quartet_index == bidi_worker_quartet_count - 1) ? list.size() : start + refs_per_quartet;
      }
      //
      for (size_t i = start; i < end; ++i) {
         auto& item = list[i];
         ++this->progress.current;
         if (!item.cell)
            continue;
         if constexpr (use_sign_masking) {
            if ((item.gx & sign_bit_mask) != x_mask)
               continue;
            if ((item.gy & sign_bit_mask) != y_mask)
               continue;
         } else {
            if ((item.gx >= 0) != this->x_pos)
               continue;
            if ((item.gy >= 0) != this->y_pos)
               continue;
         }
         item.cell->receive_inbound_ref({}, item.refr, 1, use_info_entry::flag::parent_child);
      }
   }
   //
   void load_order_persistent_ref_reparenter::bidi_worker::start() noexcept {
      assert(!this->is_active());
      this->progress.current = 0;
      this->progress.maximum = this->owner.refs.size();
      this->thread = std::thread(load_order_persistent_ref_reparenter::bidi_worker::_thread_handler, this);
   }
   void load_order_persistent_ref_reparenter::bidi_worker::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
   float load_order_persistent_ref_reparenter::bidi_worker::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
   #pragma endregion

   #pragma region load_order_persistent_ref_reparenter
   load_order_persistent_ref_reparenter::load_order_persistent_ref_reparenter() : 
      threads{ *this, *this, *this, *this },
      bidi_workers{{
         { *this, 0 },
         { *this, 1 },
      }}
   {
      this->refs.reserve(count_to_prepare_for);
   }

   void load_order_persistent_ref_reparenter::execute(file_load_order& flo) {
      flo.for_each_form_of_type(dovah::form_type::worldspace, [this](dovah::form_stub* world) -> bool {
         this->cells_for_current_world.clear();
         this->refs.clear();
         //
         auto* addenda = world->addenda;
         if (!addenda)
            return false;
         auto* p_cell = addenda->persistent_cell;
         if (!p_cell)
            return false;
         //
         this->cells_for_current_world.set_world(*world);
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
         this->refs.reserve(p_cell->inbound.size());
         {
            use_info_list entries_to_keep;
            auto& src = p_cell->inbound;
            auto& dst = entries_to_keep;
            //
            // Here, we want to do two things:
            // 
            //  - Gather up a list of all refs to reparent.
            // 
            //  - Sever the inbound connections from  the references to the persistent cell.
            //
            // 99% of the time, we will be removing every single element from the persistent 
            // cell's inbound uses. I can't even think of any circumstance in which any form 
            // would actually refer to the persistent cell for any reason other than being a 
            // child REFR of that cell. The default case is removal.
            // 
            // That in turn  means that the fastest way to filter the inbound  use map is to 
            // create a new map, copy the few (usually no) elements we intend to preserve to 
            // that new map, and then swap the two maps. Extracting and reparenting nodes is 
            // slightly slower in practice, and  using the "erase" function on the container 
            // is the slowest approach of all (which makes sense; it would only be faster if 
            // the default code path was to retain, not remove, elements).
            //
            for (auto& pair : src) {
               auto& entry = pair.second;
               if (entry.flags & use_info_entry::flag::parent_child) {
                  auto* child = entry.other;
                  if (child && form_type_is_reference(child->form_type)) {
                     this->refs.emplace_back(*child);
                     if (--entry.refcount == 0)
                        continue;
                  }
               }
               dst[pair.first] = entry;
            }
            std::swap(src, dst);
         }
         {
            size_t size = this->refs.size();
            if (size >= thread_count) {
               size_t per_thread = size / thread_count;
               size_t start      = 0;
               for (size_t i = 0; i < thread_count - 1; ++i) {
                  this->threads[i].set_range(start, start + per_thread);
                  start += per_thread;
               }
               auto& last = this->threads.back();
               last.set_range(start, size);
            } else {
               for (size_t i = 1; i < thread_count; ++i)
                  this->threads[i].set_range(0, 0);
               this->threads[0].set_range(0, size);
            }
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
         // Now, we must make those references bidirectional.
         //
         for (auto& thread : this->bidi_workers)
            thread.start();
         for (auto& thread : this->bidi_workers)
            thread.wait_for();
         return false;
      });
      this->cells_for_current_world.clear();
      this->refs.clear();
   }
   #pragma endregion
}