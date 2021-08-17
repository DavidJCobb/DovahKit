#pragma once
#include "alias.h"

namespace dovah::loaded_forms {
   class LocationAlias;
}

namespace dovahscript::wrappers {
   struct quest_loc_alias : public quest_alias {
      static constexpr const char* superclass_list = { metatable_key };
      static constexpr const char* metatable_key   = "dovah.classes.quest_loc_alias";
      static constexpr const char* class_name      = "quest_loc_alias";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_type = dovah::loaded_forms::LocationAlias;
      static wrapped_type* unwrap(wrapper& w);
   };
}