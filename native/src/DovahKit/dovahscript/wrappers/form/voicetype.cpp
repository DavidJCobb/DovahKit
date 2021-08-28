#include "voicetype.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/Voicetype.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::voicetype;
   using wrapped_type = cls::wrapped_type;
   
   namespace _methods {
   }
   namespace _getters {
      int allow_default_dialogue(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->voicetype_flags & wrapped_type::voicetype_flag::allow_default_dialogue));
         return 1;
      }
      int is_female(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->voicetype_flags & wrapped_type::voicetype_flag::female));
         return 1;
      }
   }
   namespace _setters {
      int allow_default_dialogue(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->voicetype_flags, wrapped_type::voicetype_flag::allow_default_dialogue, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int is_female(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->voicetype_flags, wrapped_type::voicetype_flag::female, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls ::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "allows_default_dialogue", &_getters::allow_default_dialogue },
      { "is_female",               &_getters::is_female },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "allows_default_dialogue", &_setters::allow_default_dialogue },
      { "is_female",               &_setters::is_female },
   };
}