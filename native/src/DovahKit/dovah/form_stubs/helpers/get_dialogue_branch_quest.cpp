#include "./get_dialogue_branch_quest.h"
#include "./get_unique_outbound_use.h"
#include "../../use_info/entry_flags/dialogue_branch.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_dialogue_branch_quest(const form_stub& subject) {
      return get_unique_outbound_use<use_info::entry_flags::dialogue_branch::parent_quest>(subject);
   }
}