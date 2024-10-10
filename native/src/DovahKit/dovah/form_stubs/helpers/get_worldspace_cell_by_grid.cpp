#include "./get_worldspace_cell_by_grid.h"
#include <cassert>
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_types.h"
#include "../../use_info_entry.h"
#include "./get_worldspace_persistent_cell.h"

namespace dovah::form_stub_helpers {
   form_stub* get_worldspace_cell_by_grid(const form_stub* world, int32_t x, int32_t y) {
      const form_stub* persistent_cell = get_worldspace_persistent_cell(world);
      for (auto& pair : world->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info_entry::flag::parent_child))
            continue;
         auto* cell = entry.other;
         if (!cell || cell->form_type != form_type::cell)
            continue;
         assert(cell->get_parent_form() == world && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
         if (cell == persistent_cell)
            continue;
         if (!cell->addenda)
            continue;
         auto& opt = cell->addenda->grid_position;
         if (!opt.has_value())
            continue;
         auto& pos = opt.value();
         if (pos.x == x && pos.y == y)
            return cell;
      }
      return nullptr;
   }
}