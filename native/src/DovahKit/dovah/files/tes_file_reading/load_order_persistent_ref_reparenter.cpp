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
}

namespace {
   int32_t _world_coords_to_grid_coords(float v) {
      if constexpr (dovah::loaded_forms::Cell::side_length == 4096) {
         //
         // If the cell size is 4096, then we can just right-shift by 0xC (sign permitting).
         //
         if constexpr (int32_t(-90) >> 2 == -23) {
            //
            // Right-shifting a negative number fills  the shifted-in bits with the sign bit 
            // on this compiler and platform. Bethesda shifts right by 0xC, so we can do the 
            // same.
            //
            return (int32_t)v >> 0xC;
         } else {
            if (v >= 0)
               return (int32_t)v >> 0xC;
            return ((int32_t)v / dovah::loaded_forms::Cell::side_length) - 1;
         }
      }
      auto out = (int32_t)v / dovah::loaded_forms::Cell::side_length;
      if (v < 0)
         --out;
      return out;
   }
}

namespace dovah::tes_file_reading {
   #pragma region load_order_persistent_ref_reparenter::cell_map
      form_stub* load_order_persistent_ref_reparenter::cell_map::cell_for_position(float x, float y) {
         static_assert(loaded_forms::Cell::side_length == 4096, "This bit-shift constant won't work.");
         auto gx = _world_coords_to_grid_coords(x);
         auto gy = _world_coords_to_grid_coords(y);
         gx += grid_halfwidth;
         gy += grid_halfwidth;
         if (gx > 0 && gx < grid_halfwidth && gy > 0 && gy < grid_halfwidth) {
            auto& item = this->sorted[gx + (gy * grid_halfwidth * 2)];
            ++item.count;
            return item.cell;
         } else {
            gx -= grid_halfwidth;
            gy -= grid_halfwidth;
            for (auto& item : this->unsorted) {
               auto* cell = item.cell;
               auto& grid = cell->addenda->grid_coords;
               if (grid.x == gx && grid.y == gy) {
                  ++item.count;
                  return cell;
               }
            }
         }
         return nullptr;
      }
      void load_order_persistent_ref_reparenter::cell_map::set_world(form_stub& world) {
         this->world = &world;
         //
         this->unsorted.clear();
         //
         auto* p_cell = world.addenda ? world.addenda->persistent_cell : nullptr;
         for (auto& pair : world.inbound) {
            auto& entry = pair.second;
            if (!(entry.flags & use_info_entry::flag::parent_child))
               continue;
            auto* cell = entry.other;
            if (!cell || cell->formType != form_type::cell)
               continue;
            assert((cell->get_parent_form() == &world) && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
            if (cell == p_cell)
               continue;
            if (!cell->addenda)
               continue;
            //
            auto gx = cell->addenda->grid_coords.x;
            auto gy = cell->addenda->grid_coords.y;
            gx += grid_halfwidth;
            gy += grid_halfwidth;
            constexpr size_t axis_size = grid_halfwidth * 2;
            constexpr size_t grid_size = axis_size * axis_size;
            //
            if (gx > 0 && gx < grid_halfwidth && gy > 0 && gy < grid_halfwidth) {
               this->sorted[gx + (gy * axis_size)].cell = cell;
               continue;
            }
            this->unsorted.emplace_back(cell);
         }
      }
      void load_order_persistent_ref_reparenter::cell_map::clear() {
         this->world = nullptr;
         this->unsorted.clear();
         for (auto& item : this->sorted) {
            item.cell = nullptr;
            item.refs.clear();
            item.count = 0;
         }
      }

      void load_order_persistent_ref_reparenter::cell_map::take_ref(form_stub* refr, int32_t gx, int32_t gy) {
         gx += grid_halfwidth;
         gy += grid_halfwidth;
         constexpr size_t axis_size = grid_halfwidth * 2;
         constexpr size_t grid_size = axis_size * axis_size;
         //
         if (gx > 0 && gx < grid_halfwidth && gy > 0 && gy < grid_halfwidth) {
            auto& item = this->sorted[gx + (gy * grid_halfwidth * 2)];
            item.refs[item.count - 1] = refr;
            item.count -= 1;
            return;
         }
         //
         gx -= grid_halfwidth;
         gy -= grid_halfwidth;
         for (auto& item : this->unsorted) {
            auto* cell = item.cell;
            auto& grid = cell->addenda->grid_coords;
            if (grid.x == gx && grid.y == gy) {
               item.refs[item.count - 1] = refr;
               item.count -= 1;
               return;
            }
         }
      }
   #pragma endregion

   #pragma region load_order_persistent_ref_reparenter::worker
   load_order_persistent_ref_reparenter::worker::worker(load_order_persistent_ref_reparenter& o) : owner(o) {
   }

   void load_order_persistent_ref_reparenter::worker::_execute() {
      auto& list = this->owner.refs;
      this->progress.maximum = this->range.end - this->range.start;
      for (size_t i = this->range.start; i < this->range.end; ++i) {
         auto& item = list[i];
         auto* stub = item.refr;
         //
         stub->_do_custom_parse(this, [this, &item](form_stub& stub, record& record, dovah::load_order_interfaces::form_load& intfc) {
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
            form_stub* cell = this->owner.cells_for_current_world.cell_for_position(x, y);
            if (cell) {
               item.cell   = cell;
               item.grid_x = cell->addenda->grid_coords.x;
               item.grid_y = cell->addenda->grid_coords.y;
               stub._set_parent_form_one_way(cell);
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
   void load_order_persistent_ref_reparenter::worker::start() {
      assert(!this->is_active());
      this->progress.current = 0;
      this->progress.maximum = this->range.end - this->range.start;
      this->thread = std::thread(load_order_persistent_ref_reparenter::worker::_thread_handler, this);
   }
   void load_order_persistent_ref_reparenter::worker::wait_for() {
      if (this->is_active())
         this->thread.join();
   }
   float load_order_persistent_ref_reparenter::worker::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
   #pragma endregion

   #pragma region bidi_worker
   void load_order_persistent_ref_reparenter::bidi_worker::_execute() {
      auto& list = this->owner.cells_for_current_world.sorted;
      for (size_t i = this->range.start; i < this->range.end; ++i) {
         auto* cell = list[i].cell;
         auto& refs = list[i].refs;
         for (auto* r : refs) {
            cell->receive_inbound_ref(r, 1, use_info_entry::flag::parent_child);
         }
      }
   }
   load_order_persistent_ref_reparenter::bidi_worker::bidi_worker(load_order_persistent_ref_reparenter& o) : owner(o) {
   }

   void load_order_persistent_ref_reparenter::bidi_worker::set_range(size_t start, size_t end) {
      this->range.start = start;
      this->range.end   = end;
   }
   void load_order_persistent_ref_reparenter::bidi_worker::start() {
      assert(!this->is_active());
      this->thread = std::thread(&_thread_handler, this);
   }
   void load_order_persistent_ref_reparenter::bidi_worker::wait_for() {
      if (this->is_active())
         this->thread.join();
   }
   #pragma endregion

   #pragma region load_order_persistent_ref_reparenter
   load_order_persistent_ref_reparenter::load_order_persistent_ref_reparenter() :
      threads{ *this, *this, *this, *this,   *this, *this, *this, *this },
      bidi_workers{ *this, *this, *this, *this,   *this, *this, *this, *this }
   {
      constexpr size_t grid_size = cell_map::grid_total_size;
      constexpr size_t list_size = std::tuple_size_v<decltype(bidi_workers)>;
      constexpr size_t cells_per = grid_size / list_size;
      for (size_t i = 0; i < list_size - 1; ++i) {
         this->bidi_workers[i].set_range(i * cells_per, (i + 1) * cells_per);
      }
      this->bidi_workers.back().set_range(
         (list_size - 1) * cells_per,
         grid_size
      );
   }

   void load_order_persistent_ref_reparenter::clear() {
      this->cells_for_current_world.clear();
      this->refs.clear();
   }

   void load_order_persistent_ref_reparenter::execute(file_load_order& flo) {
      flo.for_each_form_of_type(dovah::form_type::worldspace, [this](dovah::form_stub* world) -> bool {
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
         {
            use_info_list entries_to_keep;
            for (auto& pair : p_cell->inbound) {
               auto& entry = pair.second;
               if (entry.flags & use_info_entry::flag::parent_child) {
                  auto* child = entry.other;
                  if (child && form_type_info::form_type_is_reference(child->formType)) {
                     this->refs.push_back(child);
                     if (--entry.refcount == 0)
                        continue;
                  }
               }
               entries_to_keep[pair.first] = entry;
            }
            std::swap(p_cell->inbound, entries_to_keep);
            //
            size_t count = this->refs.size();
            if (count <= this->threads.size()) {
               this->threads[0].set_range(0, count);
               for (size_t i = 1; i < this->threads.size(); ++i) {
                  this->threads[i].set_range(0, 0);
               }
            } else {
               size_t per_thread = (count / this->threads.size());
               size_t start      = 0;
               for (size_t i = 0; i < this->threads.size() - 1; ++i) {
                  auto&  t   = this->threads[i];
                  size_t end = start + per_thread;
                  if (i == this->threads.size() - 1)
                     end = count;
                  t.set_range(start, end);
                  start += per_thread;
               }
               auto& last = this->threads.back();
               if (start < count) {
                  last.set_range(start, count);
               } else {
                  last.set_range(0, 0);
               }
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
         // Now, we must make those references bidirectional. Unfortunately, this is the 
         // step of the process that cannot be multi-threaded.
         //
         if constexpr (multithreaded_bidirectional_use_info) {
            for (auto& item : this->cells_for_current_world.unsorted)
               item.refs.resize(item.count);
            for (auto& item : this->cells_for_current_world.sorted)
               item.refs.resize(item.count);
            //
            for (auto& item : this->refs) {
               this->cells_for_current_world.take_ref(item.refr, item.grid_x, item.grid_y);
            }
            //
            for (auto& thread : this->bidi_workers)
               thread.start();
            for (auto& item : this->cells_for_current_world.unsorted) {
               for(auto& r : item.refs)
                  item.cell->receive_inbound_ref(r, 1, use_info_entry::flag::parent_child);
            }
            for (auto& thread : this->bidi_workers)
               thread.wait_for();
         } else {
            for (auto& item : this->refs) {
               assert(item.cell);
               item.cell->receive_inbound_ref(item.refr, 1, use_info_entry::flag::parent_child);
            }
         }
         this->clear();
         return false;
      });
   }
   #pragma endregion
}