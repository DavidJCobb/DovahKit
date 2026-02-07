#pragma once
#include "../condition.h"

namespace dovahscript::wrappers {
   struct condition_parameter_set : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.condition_parameter_set";
      static constexpr const char*   class_name      = "condition_parameter_set";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}