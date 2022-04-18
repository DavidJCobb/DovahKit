#include "directional_material.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Static.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::static_directional_material;
   using wrapped_type = cls::form_type::directional_material_data;

   wrapped_type* _unwrap(wrapper& w) {
      auto* form = w.get_loaded_form_data<cls::form_type>();
      if (!form)
         return nullptr;
      return &form->directional_material;
   }
}

namespace {
   namespace _getters {
      int is_snow(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         if (!data)
            return 0;
         lua_pushboolean(L, (data->flags & wrapped_type::flag::is_snow));
         return 1;
      }
      int max_angle(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         if (!data)
            return 0;
         lua_pushnumber(L, data->max_angle);
         return 1;
      }
      int material_object(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         if (!data)
            return 0;
         return push_native_object(data->material_object);
      }
   }
   namespace _setters {
      int is_snow(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!data)
            return 0;
         self.before_edit();
         cobb::edit_bit(data->flags, wrapped_type::flag::is_snow, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int material_object(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<cls::form_type>();
         auto* data  = _unwrap(self);
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::material_object);
         if (!data)
            return 0;
         self.before_edit();
         data->material_object.set(*form, value);
         self.after_edit();
         return 0;
      }
      int max_angle(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = _unwrap(self);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!data)
            return 0;
         self.before_edit();
         data->max_angle = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "is_snow",         &_getters::is_snow },
      { "material_object", &_getters::material_object },
      { "max_angle",       &_getters::max_angle },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "is_snow",         &_setters::is_snow },
      { "material_object", &_setters::material_object },
      { "max_angle",       &_setters::max_angle },
   };
}