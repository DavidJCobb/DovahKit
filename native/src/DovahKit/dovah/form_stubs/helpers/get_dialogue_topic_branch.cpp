#include "./get_dialogue_topic_branch.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./get_unique_outbound_use.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_dialogue_topic_branch(const form_stub* subject) {
      if (!subject || subject->form_type != form_type::topic)
         return nullptr;
      return get_unique_outbound_use<use_info_entry::flag::dialogue_branch>(*subject);
   }
}