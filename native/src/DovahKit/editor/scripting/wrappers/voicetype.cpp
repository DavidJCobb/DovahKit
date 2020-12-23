#include "voicetype.h"
#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/Voicetype.h"
#include "../editor_script_core.h"

namespace {
   using namespace editor_script;
   using Voicetype = dovah::loaded_forms::Voicetype;
   //
   namespace _methods {
      luastackchange_t get_allow_default_dialogue(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->voicetype_flags & Voicetype::voicetype_flag::allow_default_dialogue));
         return 1;
      }
      luastackchange_t set_allow_default_dialogue(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         cobb::edit_bit(form->voicetype_flags, Voicetype::voicetype_flag::allow_default_dialogue, lua_toboolean(L, 2));
         return 0;
      }
      luastackchange_t get_is_female(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->voicetype_flags & Voicetype::voicetype_flag::female));
         return 1;
      }
      luastackchange_t set_is_female(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         cobb::edit_bit(form->voicetype_flags, Voicetype::voicetype_flag::female, lua_toboolean(L, 2));
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ luaL_Reg voicetype::metatable_methods[] = no_registrations;
   
   /*static*/ luaL_Reg voicetype::metatable_getters[] = {
      { "allows_default_dialogue", &_methods::get_allow_default_dialogue },
      { "is_female",               &_methods::get_is_female },
      { nullptr, nullptr },
   };
   /*static*/ luaL_Reg voicetype::metatable_setters[] = {
      { "allows_default_dialogue", &_methods::set_allow_default_dialogue },
      { "is_female",               &_methods::set_is_female },
      { nullptr, nullptr },
   };
}