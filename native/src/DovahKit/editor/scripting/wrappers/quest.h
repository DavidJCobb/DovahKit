#pragma once
#include "form.h"

#include "../../../dovah/forms/Quest.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc quest_alias       = "QstAlias"; // only used for the collection
   inline constexpr cobb::eight_cc quest_alias_by_id = "QstAlsID"; // used for the by-ID collection and for all aliases in both collections
}

namespace editor_script::wrappers {
   struct quest : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.quest";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* alias_collection_key    = "collection<dovah.classes.quest.aliases>";
      static constexpr const char* alias_id_collection_key = "collection<dovah.classes.quest.aliases_by_id>";
      static void build_collection_metatables(lua_State* L);
   };
}