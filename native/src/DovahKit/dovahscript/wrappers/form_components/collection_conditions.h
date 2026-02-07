#pragma once
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
}
namespace dovahscript::wrappers::collections {
   extern const collection_definition_params condition_list;

   extern dovah::loaded_forms::components::condition_list* unwrap_condition_list(wrapper&);
}