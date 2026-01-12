#include "./get_cell_landscape.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_types.h"
#include "../../use_info/entry.h"
#include "../../use_info/entry_flag_to_mask.h"
#include "../../use_info/entry_flags/base.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_cell_landscape(const form_stub& cell) {
      if (cell.form_type != dovah::form_type::cell)
         return nullptr;
      if (auto* addenda = cell.addenda)
         if (auto* land = addenda->canonical_landscape)
            return land;
      for (auto& pair : cell.inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)) {
            auto* child = entry.other;
            if (!child)
               continue;
            if (child->form_type == dovah::form_type::land)
               return child;
         }
      }
      return nullptr;
   }
}