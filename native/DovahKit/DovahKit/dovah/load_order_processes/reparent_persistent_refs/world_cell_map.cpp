#include "./world_cell_map.h"
#include <cassert>
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovah/form_type.h"
#include "dovah/use_info/entry_flags/base.h"
#include "dovah/use_info/entry_flag_to_mask.h"
#include "dovah/utils/form_stubs/get_parent_form.h"
#include "dovah/utils/worldspaces/world_position_to_grid_position.h"

namespace dovah::load_order_processes::reparent_persistent_refs {
   form_stub* world_cell_map::cell_for_position(float x, float y) const {
      auto [gx, gy] = utils::worldspaces::world_position_to_grid_position(x, y);
      if (auto* item = this->cells.grid.at(gx, gy)) {
         return *item;
      }
      for (auto* cell : this->cells.beyond) {
         const auto& sg = cell->addenda->grid_position;
         if (!sg.has_value())
            continue;
         if (sg.value().x == gx && sg.value().y == gy)
            return cell;
      }
      return nullptr;
   }
   void world_cell_map::set_world(form_stub& world) {
      this->world = &world;
      auto* p_cell = world.addenda ? world.addenda->persistent_cell : nullptr;
      for (auto& pair : world.inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)))
            continue;
         auto* cell = entry.other;
         if (!cell || cell->form_type != form_type::cell)
            continue;
         assert((utils::form_stubs::get_parent_form(*cell) == &world) && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
         if (cell == p_cell)
            continue;
         if (!cell->addenda || !cell->addenda->grid_position.has_value())
            continue;

         auto gx = cell->addenda->grid_position.value().x;
         auto gy = cell->addenda->grid_position.value().y;
         if (auto* item = this->cells.grid.at(gx, gy))
            *item = cell;
         else
            this->cells.beyond.push_back(cell);
      }
   }

   void world_cell_map::clear() {
      this->world = nullptr;
      this->cells = {};
   }
}