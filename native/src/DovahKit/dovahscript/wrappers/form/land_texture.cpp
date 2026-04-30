#include "./land_texture.h"
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/LandTexture.h"
#include "./land_texture/collection_grasses.h"
#include "./land_texture/havok.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::land_texture;
   using wrapped_type = cls::wrapped_type;
}

namespace {
   namespace _getters {
      int physics(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::land_texture_physics);
         return core::subsystems::userdata::get().push(L, out, wrappers::land_texture_havok::metatable_key);
      }
      int grasses(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return wrapper_likes::native_lists::land_texture_grasses::push(L, self);
      }
      int specular_exponent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->specular_exponent);
         return 1;
      }
      int texture_set(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->texture_set);
      }
   }
   namespace _setters {
      int specular_exponent(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,      2, "integer expected");
         luaL_argcheck(L, value >= 0, 2, "the specular exponent cannot be negative");
         luaL_argcheck(L, value < 31, 2, "the specular exponent cannot exceed 30");
         if (!form)
            return 0;
         self.before_edit();
         form->specular_exponent = value;
         self.after_edit();
         return 0;
      }
      int texture_set(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::texture_set);
         if (!form)
            return 0;
         self.before_edit();
         form->texture_set.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "grasses",           &_getters::grasses },
      { "physics",           &_getters::physics },
      { "specular_exponent", &_getters::specular_exponent },
      { "texture_set",       &_getters::texture_set },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "specular_exponent", &_setters::specular_exponent },
      { "texture_set",       &_setters::texture_set },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      wrapper_likes::native_lists::land_texture_grasses::define_metatable(L);
   }
}