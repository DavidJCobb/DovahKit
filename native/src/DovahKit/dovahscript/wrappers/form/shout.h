#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Shout;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc shout_word = "ShoutWrd";
}

namespace dovahscript::wrappers {
   struct shout : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.shout";
      static constexpr const char*   class_name      = "shout";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Shout;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;
   };
}