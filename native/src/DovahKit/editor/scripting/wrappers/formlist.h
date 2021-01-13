#pragma once
#include "form.h"

#include "../../../dovah/forms/FormList.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc formlist_entries = "FormList";
}

namespace editor_script::wrappers {
   struct formlist : public form {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.formlist";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr char* entry_collection_key = "collection<dovah.classes.formlist.entries>";
      static void build_collection_metatables(lua_State* L);
   };
}