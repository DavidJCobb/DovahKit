#include "word_of_power.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/WordOfPower.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::word_of_power;
   using _loaded_form_t = dovah::loaded_forms::WordOfPower;
   //
   namespace _getters {
      luastackchange_t dragon_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->dragon_name.c_str());
         return 1;
      }
      luastackchange_t human_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->human_name.c_str());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t dragon_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->dragon_name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
      luastackchange_t human_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->human_name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "dragon_name", &_getters::dragon_name },
      { "human_name",  &_getters::human_name },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "dragon_name", &_setters::dragon_name },
      { "human_name",  &_setters::human_name },
   };
}