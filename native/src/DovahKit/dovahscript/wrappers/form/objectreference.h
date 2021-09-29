#pragma once
#include "form.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc refr_position = "RefrPosi";
   inline constexpr cobb::eight_cc refr_rotation = "RefrRota";
}

namespace dovahscript::wrappers {
   struct objectreference : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.objectreference";
      static constexpr const char*   class_name      = "objectreference";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}