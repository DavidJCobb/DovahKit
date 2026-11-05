#include "./get_parent_form.h"
#include "dovah/form_stub.h"
#include "dovah/form_use_info/entry_flags/base.h"
#include "dovah/form_use_info/entry_flag_to_mask.h"

namespace dovah::utils::form_stubs {
   extern form_stub* get_parent_form(const form_stub& child) {
      for (auto& pair : child.outbound) {
         auto& entry = pair.second;
         if (!entry.other)
            continue;
         if (!(entry.flags & form_use_info::entry_flag_to_mask(form_use_info::entry_flags::base::parent)))
            continue;
         return entry.other;
      }
      return nullptr;
   }
}