#include "./update_large_ref_index.h"
#include "../forms/_component_access.h" // to get OBND from base form
#include "../forms/MovableStatic.h"
#include "../forms/ObjectReference.h"
#include "../form_stub.h"
#include "../form_stubs/helpers/for_each_child_form.h"
#include "../form_stubs/helpers/get_base_form.h"
#include "../form_stubs/helpers/get_worldspace_cell_by_grid.h"
#include "../form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "./world_coordinate_to_grid_coordinate.h"

namespace dovah::utils {
   bool update_large_ref_index::_base_form_type_can_ever_be_eligible(dovah::form_type ft) const {
      switch (ft) {
         case dovah::form_type::statik:
            return true;
         case dovah::form_type::movable_static:
            return this->current_game != game::skyrim_special;
      }
      return false;
   }
   bool update_large_ref_index::_loaded_base_form_is_eligible(const dovah::loaded_forms::Form& loaded) const {
      switch (loaded.stub.form_type) {
         case dovah::form_type::movable_static:
            {
               auto& casted = (const dovah::loaded_forms::MovableStatic&)loaded;
               if (!(casted.flags & loaded_forms::MovableStatic::flag::is_static))
                  break;
            }
            return true;
      }
      return true;
   }
   float update_large_ref_index::_base_form_size(form_stub& stub) {
      auto& cache = this->_cache.base_form_sizes;
      {
         auto it = cache.find(&stub);
         if (it != cache.end())
            return it->second;
      }

      float size = 0.0F;
      {
         auto loaded_ptr = stub.load();
         if (loaded_ptr && _loaded_base_form_is_eligible(*loaded_ptr)) {
            auto* bounds = loaded_forms::component_access::get_object_bounds(loaded_ptr);
            if (bounds) {
               size = cobb::vector3<float>(bounds->max - bounds->min).length();
            }
         }
      }
      cache[&stub] = size;
      return size;
   }
   std::optional<update_large_ref_index::ref_sizing_info> update_large_ref_index::_large_ref_size(form_stub& ref) {
      if (ref.is_deleted())
         return {};

      auto* base_stub = form_stub_helpers::get_base_form(ref);
      if (!base_stub)
         return {};
      if (!_base_form_type_can_ever_be_eligible(base_stub->form_type))
         return {};
      float base_size = _base_form_size(*base_stub);
      if (base_size <= 0.0F)
         return {};

      ref_sizing_info info;

      float ref_scale;
      {
         auto loaded_ptr = ref.load().ptr_cast<loaded_forms::ObjectReference>();
         if (!loaded_ptr)
            return {};
         ref_scale     = loaded_ptr->get_scale();
         info.position = loaded_ptr->position;
      }

      info.radius = base_size * ref_scale;
      if (info.radius >= this->_cache.large_ref_min_size)
         return info;

      return {};
   }

   void update_large_ref_index::_scan_from_worldspace() {
      auto* persistent_cell = form_stub_helpers::get_worldspace_persistent_cell(*this->worldspace);
      form_stub_helpers::for_each_child_form(*this->worldspace, [this, persistent_cell](form_stub& cell) {
         if (cell.form_type != dovah::form_type::cell)
            return;
         std::optional<cell_grid_dword> parent_cell_grid_opt;
         if (&cell != persistent_cell) {
            int32_t x;
            int32_t y;
            if (cell.get_grid_coordinates(x, y)) {
               auto& pcg = parent_cell_grid_opt.emplace();
               pcg.x = x;
               pcg.y = y;
               if (x != (int16_t)x || y != (int16_t)y) {
                  //
                  // This cell's grid coordinates are not representable in a dword, so don't bother 
                  // listing its refs. (This should never actually happen; the engine will break in 
                  // other ways long before you get 32768 cells out from the world origin. The CK 
                  // doesn't even check for this.)
                  //
                  return;
               }
            }
         }
         form_stub_helpers::for_each_child_form(cell, [this, &parent_cell_grid_opt](form_stub& child) {
            if (!dovah::form_type_is_reference(child.form_type))
               return;
            if (!child.is_edited_or_in_active_file())
               return;
            auto sizing_opt = _large_ref_size(child);
            if (!sizing_opt.has_value())
               return;
            auto& sizing = sizing_opt.value();

            int32_t min_x = world_coordinate_to_grid_coordinate(sizing.position.x - sizing.radius);
            int32_t max_x = world_coordinate_to_grid_coordinate(sizing.position.x + sizing.radius);
            int32_t min_y = world_coordinate_to_grid_coordinate(sizing.position.y - sizing.radius);
            int32_t max_y = world_coordinate_to_grid_coordinate(sizing.position.y + sizing.radius);

            // Skip the ref if none of the cells it overlaps have grid coordinates representable in 
            // a dword. (Again, this should be impossible, and the CK doesn't bother to check for 
            // it.)
            {
               constexpr const auto min_allowed = std::numeric_limits<int16_t>::lowest();
               constexpr const auto max_allowed = std::numeric_limits<int16_t>::max();
               if (min_x > max_allowed || max_x < min_allowed) {
                  return;
               }
               if (min_x < min_allowed) {
                  min_x = min_allowed; // clamp
               }
               if (max_x > max_allowed) {
                  max_x = max_allowed; // clamp
               }
            }

            bool pcg_had_value = parent_cell_grid_opt.has_value();
            if (!pcg_had_value) {
               auto& pcg = parent_cell_grid_opt.emplace();
               pcg.x = world_coordinate_to_grid_coordinate(sizing.position.x);
               pcg.y = world_coordinate_to_grid_coordinate(sizing.position.y);
               if (pcg.x != (int16_t)pcg.x || pcg.y != (int16_t)pcg.y)
                  return;
            }
            {
               const auto& pcg = parent_cell_grid_opt.value();
               for (int32_t x = min_x; x <= max_x; ++x) {
                  for (int32_t y = min_y; y <= max_y; ++y) {
                     if (!form_stub_helpers::get_worldspace_cell_by_grid(*this->worldspace, x, y)) // CK skips coords for non-existent cells
                        continue;
                     cell_grid_dword grid{ .y = (int16_t)y, .x = (int16_t)x };
                     auto& list = this->cells_to_refs[grid];
                     auto& item = list.emplace_back();
                     item.ref = &child;
                     list.push_back(ref_info{
                        .ref         = &child,
                        .parent_cell = pcg,
                     });
                  }
               }
            }
            if (!pcg_had_value) {
               parent_cell_grid_opt = {};
            }
         });
      });
   }

   void update_large_ref_index::gather(form_stub& worldspace) {
      assert(worldspace.form_type == dovah::form_type::worldspace);
      this->worldspace = &worldspace;
      this->cells_to_refs.clear();
      
      {
         auto& dst = this->_cache.large_ref_min_size;
         auto& flo = worldspace.get_owning_load_order();

         dovah::loaded_game_setting loaded;
         if (flo.get_loaded_setting_by_name("fLargeRefMinSize", loaded))
            dst = loaded.value.f;
         else {
            auto* dfn = game_setting_definition::lookup("fLargeRefMinSize");
            if (dfn)
               dst = dfn->default_value.f;
            else
               dst = 1024.0F;
         }
      }

      this->_scan_from_worldspace();
   }
   void update_large_ref_index::apply(loaded_forms::Form& dst_owner, large_ref_index& dst) {
      dst.clear(dst_owner);
      for (const auto& [cell, src_refs] : this->cells_to_refs) {
         auto& dst_refs = dst.cells_to_refs[cell];
         size_t size = src_refs.size();
         dst_refs.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_item = src_refs[i];
            auto& dst_item = dst_refs[i];
            dst_item.form.set(dst_owner, src_item.ref);
            dst_item.parent_cell_id = src_item.parent_cell;
         }
      }
   }
}