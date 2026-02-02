#pragma once
#include "../condition.h"

namespace dovahscript::wrappers {
   struct condition_comparison : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.condition_comparison";
      static constexpr const char*   class_name      = "condition_comparison";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}