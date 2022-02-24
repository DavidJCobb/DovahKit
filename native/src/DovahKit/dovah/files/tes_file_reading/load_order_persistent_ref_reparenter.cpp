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
   #pragma region load_order_persistent_ref_reparenter::cached_cell_list
      #pragma region ...::by_grid
         form_stub* load_order_persistent_ref_reparenter::cached_cell_list::by_grid::lookup(int32_t gx, int32_t gy) const {
            if (gx > this->bounds.max.x)
               return nullptr;
            if (gy > this->bounds.max.y)
               return nullptr;
            if constexpr (pre_listed_cell_grid_can_shrink) {
               gx -= this->bounds.min.x;
               gy -= this->bounds.min.y;
            } else {
               gx += max_pre_sort_world_grid;
               gy += max_pre_sort_world_grid;
            }
            if (gx < 0 || gy < 0)
               return nullptr;
            if constexpr (pre_listed_cell_grid_can_shrink) {
               auto w = (size_t)(this->bounds.max.x - this->bounds.min.x) + 1;
               return this->data[(gy * w) + gx];
            } else {
               return this->data[(gy * (max_pre_sort_world_grid * 2)) + gx];
            }
         }
      #pragma endregion
      //
      form_stub* load_order_persistent_ref_reparenter::cached_cell_list::cell_for_position(float x, float y) const {
         static_assert(loaded_forms::Cell::side_length == 4096, "This bit-shift constant won't work.");
         auto gx = _world_coords_to_grid_coords(x);
         auto gy = _world_coords_to_grid_coords(y);
         if constexpr (!pre_list_world_cells) {
            return form_stub_helpers::get_worldspace_cell_by_grid(this->world, gx, gy);
         } else {
            if constexpr (max_pre_sort_world_grid > 0) {
               auto* cell = this->sorted.lookup(gx, gy);
               if (cell)
                  return cell;
            }
            for (auto* cell : this->unsorted) {
               auto& sg = cell->addenda->grid_coords;
               if (sg.x == gx && sg.y == gy)
                  return cell;
            }
         }
         return nullptr;
      }
      void load_order_persistent_ref_reparenter::cached_cell_list::set_world(form_stub& world) {
         this->world = &world;
         if constexpr (pre_list_world_cells) {
            this->unsorted.clear();
            if constexpr (max_pre_sort_world_grid > 0) {
               if constexpr (pre_listed_cell_grid_can_shrink) {
                  this->sorted.data.reserve(max_pre_sort_world_grid * 2 * max_pre_sort_world_grid * 2);
               } else {
                  constexpr size_t axis_size = max_pre_sort_world_grid * 2;
                  constexpr size_t grid_size = axis_size * axis_size;
                  //
                  this->sorted.data.resize(grid_size);
               }
            }
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
               if constexpr (max_pre_sort_world_grid > 0) {
                  auto gx = cell->addenda->grid_coords.x;
                  auto gy = cell->addenda->grid_coords.y;
                  if constexpr (pre_listed_cell_grid_can_shrink) {
                     this->unsorted.push_back(cell);
                     auto& bounds = this->sorted.bounds;
                     if (by_grid::range::can_represent(gx) && gx >= -max_pre_sort_world_grid && gx <= max_pre_sort_world_grid) {
                        if (gx < bounds.min.x)
                           bounds.min.x = gx;
                        if (gx > bounds.max.x)
                           bounds.max.x = gx;
                     }
                     if (by_grid::range::can_represent(gy) && gy >= -max_pre_sort_world_grid && gy <= max_pre_sort_world_grid) {
                        if (gy < bounds.min.y)
                           bounds.min.y = gy;
                        if (gy > bounds.max.y)
                           bounds.max.y = gy;
                     }
                  } else {
                     constexpr size_t axis_size = max_pre_sort_world_grid * 2;
                     constexpr size_t grid_size = axis_size * axis_size;
                     //
                     gx += max_pre_sort_world_grid;
                     gy += max_pre_sort_world_grid;
                     if (gx >= 0 && gy >= 0) {
                        if (gx < axis_size && gy < axis_size) {
                           this->sorted.data[gx + (gy * axis_size)] = cell;
                           continue;
                        }
                     }
                     this->unsorted.push_back(cell);
                  }
               }
            }
            if constexpr (max_pre_sort_world_grid > 0 && pre_listed_cell_grid_can_shrink) {
               this->sorted.data.clear();
               //
               bool  took_any = false;
               auto& bounds   = this->sorted.bounds;
               assert(bounds.max.x >= bounds.min.x);
               assert(bounds.max.y >= bounds.min.y);
               //
               size_t row_size  = (size_t)(bounds.max.x - bounds.min.x) + 1;
               size_t grid_size = row_size * ((size_t)(bounds.max.y - bounds.min.y) + 1);
               this->sorted.data.resize(grid_size);
               //
               for (auto*& item : this->unsorted) {
                  auto gx = item->addenda->grid_coords.x;
                  auto gy = item->addenda->grid_coords.y;
                  if (gx >= bounds.min.x && gx <= bounds.max.x) {
                     if (gy >= bounds.min.y && gy <= bounds.max.y) {
                        took_any = true;
                        //
                        gx -= bounds.min.x;
                        gy -= bounds.min.y;
                        std::swap(this->sorted.data[gx + (gy * row_size)], item);
                     }
                  }
               }
               if (took_any) {
                  std::erase(this->unsorted, nullptr);
               }
            }
         }
      }
      void load_order_persistent_ref_reparenter::cached_cell_list::clear() {
         this->world = nullptr;
         if constexpr (pre_list_world_cells) {
            this->unsorted.clear();
            if constexpr (max_pre_sort_world_grid > 0) {
               this->sorted.bounds = decltype(by_grid::bounds)();
               this->sorted.data.clear();
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
               item.cell = cell;
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

   #pragma region quadrant_bidi_worker
   void load_order_persistent_ref_reparenter::quadrant_bidi_worker::_execute() {
      auto& list = this->owner.refs;
      for (auto& item : list) {
         auto& grid = item.cell->addenda->grid_coords;
         if (this->x_pos != (grid.x > 0))
            continue;
         if (this->y_pos != (grid.y > 0))
            continue;
         item.cell->receive_inbound_ref(item.refr, 1, use_info_entry::flag::parent_child);
      }
   }
   load_order_persistent_ref_reparenter::quadrant_bidi_worker::quadrant_bidi_worker(load_order_persistent_ref_reparenter& o, bool x_pos, bool y_pos) : owner(o), x_pos(x_pos), y_pos(y_pos) {
   }

   void load_order_persistent_ref_reparenter::quadrant_bidi_worker::start() {
      assert(!this->is_active());
      this->thread = std::thread(&_thread_handler, this);
   }
   void load_order_persistent_ref_reparenter::quadrant_bidi_worker::wait_for() {
      if (this->is_active())
         this->thread.join();
   }
   #pragma endregion

   #pragma region load_order_persistent_ref_reparenter
   load_order_persistent_ref_reparenter::load_order_persistent_ref_reparenter() :
      threads{ *this, *this, *this, *this },
      bidi_workers{{
         { *this, true,  true  },
         { *this, true,  false },
         { *this, false, true  },
         { *this, false, false },
      }}
   {
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
               size_t per_thread = (count / this->threads.size()) + 1;
               size_t start      = 0;
               for (size_t i = 0; i < this->threads.size(); ++i) {
                  auto&  t   = this->threads[i];
                  size_t end = start + per_thread;
                  if (i == this->threads.size() - 1)
                     end = this->refs.size();
                  t.set_range(start, end);
                  start += per_thread;
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
            for (auto& thread : this->bidi_workers)
               thread.start();
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