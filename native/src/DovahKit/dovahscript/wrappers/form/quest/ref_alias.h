#pragma once
#include "alias.h"

namespace dovah::loaded_forms {
   class ReferenceAlias;
}

namespace dovahscript::wrappers {
   struct quest_ref_alias : public quest_alias {
      static constexpr const char* superclass_list = { metatable_key };
      static constexpr const char* metatable_key   = "dovah.classes.quest_ref_alias";
      static constexpr const char* class_name      = "quest_ref_alias";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_type = dovah::loaded_forms::ReferenceAlias;
      static wrapped_type* unwrap(wrapper& w);
   };
}