#pragma once
#include <utility> // std::pair
#include "helpers/eight_cc.h"
#include "dovahscript/core/collections.h"
namespace dovah::loaded_forms::components {
   class condition_list;
}
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc condition_list = "CtdnList";

   // Specific condition lists, on the handful of forms that have multiple:
   inline constexpr cobb::eight_cc condition_list_quest_dialogue = "CtdLQDia";
   inline constexpr cobb::eight_cc condition_list_quest_events   = "CtdLQEvt";
}
namespace dovahscript::wrappers::collections {
   extern const collection_definition_params condition_list;

   extern dovah::loaded_forms::components::condition_list* unwrap_condition_list(wrapper&);
   extern std::pair<dovah::loaded_forms::components::condition_list*, size_t> unwrap_condition_list_and_index(wrapper&);
}