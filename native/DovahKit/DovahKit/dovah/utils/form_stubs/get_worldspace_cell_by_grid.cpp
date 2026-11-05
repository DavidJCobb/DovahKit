#include "./get_worldspace_cell_by_grid.h"
#include <cassert>
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_type.h"
#include "../../form_use_info/entry_flag_to_mask.h"
#include "../../form_use_info/entry_flags/base.h"
#include "./get_parent_form.h"
#include "./get_worldspace_persistent_cell.h"

namespace dovah::utils::form_stubs {
   extern form_stub* get_worldspace_cell_by_grid(const form_stub& world, int32_t gx, int32_t gy) {
      const form_stub* persistent_cell = get_worldspace_persistent_cell(world);
      for (auto& pair : world.inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & form_use_info::entry_flag_to_mask(form_use_info::entry_flags::base::parent)))
            continue;
         auto* cell = entry.other;
         if (!cell || cell->form_type != form_type::cell)
            continue;
         assert(get_parent_form(*cell) == &world && "How did a worldspace form a parent/child relationship with a cell that doesn't consider that world its parent?");
         if (cell == persistent_cell)
            continue;
         if (!cell->addenda)
            continue;
         auto& opt = cell->addenda->grid_position;
         if (!opt.has_value())
            continue;
         auto& pos = opt.value();
         if (pos.x == gx && pos.y == gy)
            return cell;
      }
      return nullptr;
   }
}