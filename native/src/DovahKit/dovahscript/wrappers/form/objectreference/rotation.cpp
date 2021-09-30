#include "rotation.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/rotation.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/ObjectReference.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::objectreference_rotation;
   using wrapped_type = dovah::loaded_forms::ObjectReference;

   namespace _getters {
      int x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->rotation.x);
         return 1;
      }
      int y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->rotation.y);
         return 1;
      }
      int z(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->rotation.z);
         return 1;
      }
   }
   namespace _setters {
      int x(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->rotation.x = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int y(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->rotation.y = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int z(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->rotation.z = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
      { "z", &_getters::z },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "x", &_setters::x },
      { "y", &_setters::y },
      { "z", &_setters::z },
   };
}