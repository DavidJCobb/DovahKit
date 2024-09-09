#include "./get_dialogue_branch_quest.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./get_unique_outbound_use.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_dialogue_branch_quest(const form_stub* subject) {
      if (!subject || subject->form_type != form_type::dialogue_branch)
         return nullptr;
      return get_unique_outbound_use<use_info_entry::flag::dialogue_quest>(*subject);
   }
}