#pragma once
#include "form.h"

#include "../../../dovah/forms/Shout.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc shout_word = "ShoutWrd";
}

namespace editor_script::wrappers {
   struct shout : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.shout";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* word_collection_key = "collection<dovah.classes.shout.words>";
      static void build_collection_metatables(lua_State* L);
   };
}