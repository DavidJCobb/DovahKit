#pragma once
#include "../../wrapper.h"
#include "../../../dovah/forms/Quest.h"

namespace editor_script::wrappers {
   struct quest_alias : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.quest_alias";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::Alias;
      static wrapped_t* unwrap(wrapper& w);

      static luastackchange_t wrap(lua_State* L, dovah::form_stub* quest, uint32_t aliasID);
      static luastackchange_t wrap(lua_State* L, dovah::form_stub* quest, const wrapped_t* alias);
      static luastackchange_t wrap(lua_State* L, const wrapper& collection, const wrapped_t* alias);
   };
   
   struct quest_loc_alias : public quest_alias {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.quest_loc_alias";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::LocationAlias;
      static wrapped_t* unwrap(wrapper& w);
   };

   struct quest_ref_alias : public quest_alias {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.quest_ref_alias";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::ReferenceAlias;
      static wrapped_t* unwrap(wrapper& w);
   };
}