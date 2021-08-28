#include "word_of_power.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/WordOfPower.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::word_of_power;
   using wrapped_type = cls::wrapped_type;

   namespace _getters {
      int dragon_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->dragon_name.c_str());
         return 1;
      }
      int human_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->human_name.c_str());
         return 1;
      }
   }
   namespace _setters {
      int dragon_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->dragon_name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
      int human_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->human_name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "dragon_name", &_getters::dragon_name },
      { "human_name",  &_getters::human_name },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "dragon_name", &_setters::dragon_name },
      { "human_name",  &_setters::human_name },
   };
}