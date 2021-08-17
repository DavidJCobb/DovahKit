#pragma once
#include "form.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc quest_alias       = "QstAlias"; // only used for the collection
   inline constexpr cobb::eight_cc quest_alias_by_id = "QstAlsID"; // used for the by-ID collection and for all aliases in both collections
}

namespace dovahscript::wrappers {
   struct quest : public form {
      static constexpr const char* superclass_list = { metatable_key };
      static constexpr const char* metatable_key   = "dovah.classes.quest";
      static constexpr const char* class_name      = "quest";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;
   };
}