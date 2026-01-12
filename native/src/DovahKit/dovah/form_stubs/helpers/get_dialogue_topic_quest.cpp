#include "./get_dialogue_topic_quest.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info/entry_flags/topic.h"
#include "./get_unique_outbound_use.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_dialogue_topic_quest(const form_stub& subject) {
      if (subject.form_type != form_type::topic)
         return nullptr;
      return get_unique_outbound_use<use_info::entry_flags::topic::parent_quest>(subject);
   }
}