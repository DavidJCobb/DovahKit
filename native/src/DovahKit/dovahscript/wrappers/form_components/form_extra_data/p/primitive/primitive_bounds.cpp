#include "./primitive_bounds.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/wrapper.h"
#include "../primitive.h"

#include "dovah/forms/components/extra_data/types/p/primitive.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::form_extra_data_types::primitive__bounds;

   cobb::vector3<float>& _unwrap_self(lua_State* L) {
      auto& self = get_wrapper_for_thiscall<cls>(L);
      auto* data = wrappers::form_extra_data_types::primitive::unwrap(self);
      if (!data)
         cobb::lua::argerror(L, 1, "no underlying object");
      return data->bounds;
   }

   namespace _methods {
      int set_xyz(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_self(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number (x) expected");
         cobb::lua::argcheck(L, lua_isnumber(L, 3), 3, "number (y) expected");
         cobb::lua::argcheck(L, lua_isnumber(L, 4), 4, "number (z) expected");
         float x = lua_tonumber(L, 2);
         float y = lua_tonumber(L, 3);
         float z = lua_tonumber(L, 4);
         self.before_edit();
         data = { x, y, z };
         self.after_edit();
         return 0;
      }
   }
   namespace _getters {
      int x(lua_State* L) {
         auto& data = _unwrap_self(L);
         lua_pushnumber(L, data.x);
         return 1;
      }
      int y(lua_State* L) {
         auto& data = _unwrap_self(L);
         lua_pushnumber(L, data.y);
         return 1;
      }
      int z(lua_State* L) {
         auto& data = _unwrap_self(L);
         lua_pushnumber(L, data.z);
         return 1;
      }
   }
   namespace _setters {
      int x(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_self(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");

         self.before_edit();
         data.x = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int y(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_self(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");

         self.before_edit();
         data.y = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int z(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto& data = _unwrap_self(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");

         self.before_edit();
         data.z = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "set_xyz", &_methods::set_xyz },
   };
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