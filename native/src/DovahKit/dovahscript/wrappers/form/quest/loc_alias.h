#pragma once
#include "alias.h"

namespace dovah::loaded_forms {
   class LocationAlias;
}

namespace dovahscript::wrappers {
   struct quest_loc_alias : public quest_alias {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.quest_loc_alias";
      static constexpr const char*   class_name      = "quest_loc_alias";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::LocationAlias;
      static wrapped_type* unwrap(wrapper& w);
   };
}