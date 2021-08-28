#include "havok.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/LandTexture.h"
#include "../land_texture.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::land_texture_havok;
   using wrapped_type = dovah::loaded_forms::LandTexture;
}

namespace {
   namespace _getters {
      int friction(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->havok.friction);
         return 1;
      }
      int material(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->havok.material);
      }
      int restitution(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->havok.restitution);
         return 1;
      }
   }
   namespace _setters {
      int friction(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,       2, "integer expected");
         luaL_argcheck(L, value >=  0, 2, "land textures cannot have negative friction");
         luaL_argcheck(L, value < 256, 2, "land textures cannot have friction above 255");
         if (!form)
            return 0;
         self.before_edit();
         form->havok.friction = value;
         self.after_edit();
         return 0;
      }
      int material(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::material_type);
         if (!form)
            return 0;
         self.before_edit();
         form->havok.material.set(*form, value);
         self.after_edit();
         return 0;
      }
      int restitution(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,       2, "integer expected");
         luaL_argcheck(L, value >=  0, 2, "land textures cannot have negative restitution");
         luaL_argcheck(L, value < 256, 2, "land textures cannot have restitution above 255");
         if (!form)
            return 0;
         self.before_edit();
         form->havok.restitution = value;
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "friction",    &_getters::friction },
      { "material",    &_getters::material },
      { "restitution", &_getters::restitution },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "friction",    &_setters::friction },
      { "material",    &_setters::material },
      { "restitution", &_setters::restitution },
   };
}