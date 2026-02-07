#pragma once
#include "helpers/eight_cc.h"
#include "../base.h"
namespace dovah::loaded_forms::components {
   namespace conditions {
      struct context;
   }
   class condition;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc condition_comparison = "CdtnCmps";
   inline constexpr cobb::eight_cc condition_parameters = "CdtnArgs";
}
namespace dovahscript::wrappers {
   struct condition : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.condition";
      static constexpr const char*   class_name      = "condition";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::components::condition;
      using context_type = dovah::loaded_forms::components::conditions::context;

      static wrapped_type* unwrap(wrapper&);
      static context_type  context_of(wrapper&);
   };
}