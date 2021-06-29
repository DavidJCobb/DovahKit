#include "voicetype.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/Voicetype.h"

namespace {
   using namespace editor_script;
   using Voicetype = dovah::loaded_forms::Voicetype;
   //
   namespace _methods {
   }
   namespace _getters {
      luastackchange_t allow_default_dialogue(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->voicetype_flags & Voicetype::voicetype_flag::allow_default_dialogue));
         return 1;
      }
      luastackchange_t is_female(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->voicetype_flags & Voicetype::voicetype_flag::female));
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t allow_default_dialogue(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->voicetype_flags, Voicetype::voicetype_flag::allow_default_dialogue, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      luastackchange_t is_female(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrappers::voicetype>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<Voicetype>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->voicetype_flags, Voicetype::voicetype_flag::female, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> voicetype::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> voicetype::metatable_getters = {
      { "allows_default_dialogue", &_getters::allow_default_dialogue },
      { "is_female",               &_getters::is_female },
   };
   /*static*/ std::initializer_list<luaL_Reg> voicetype::metatable_setters = {
      { "allows_default_dialogue", &_setters::allow_default_dialogue },
      { "is_female",               &_setters::is_female },
   };
}