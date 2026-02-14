#pragma once
#include "alias.h"

namespace dovah::loaded_forms {
   class ReferenceAlias;
}

namespace dovahscript::wrappers {
   struct quest_ref_alias : public quest_alias {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.quest_ref_alias";
      static constexpr const char*   class_name      = "quest_ref_alias";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);

      using wrapped_type = dovah::loaded_forms::ReferenceAlias;
      static wrapped_type* unwrap(wrapper& w);
   };
}