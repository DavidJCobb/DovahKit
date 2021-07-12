#include "havok.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../wrapper_util.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::land_texture_havok;
   using _loaded_form_t = dovah::loaded_forms::LandTexture;
}

namespace {
   namespace _getters {
      luastackchange_t friction(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->havok.friction);
         return 1;
      }
      luastackchange_t material(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->havok.material);
      }
      luastackchange_t restitution(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->havok.restitution);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t friction(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
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
      luastackchange_t material(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::material_type);
         if (!form)
            return 0;
         self.before_edit();
         form->havok.material.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t restitution(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
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
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "friction",    &_getters::friction },
      { "material",    &_getters::material },
      { "restitution", &_getters::restitution },
   };
   /*static*/ const std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "friction",    &_setters::friction },
      { "material",    &_setters::material },
      { "restitution", &_setters::restitution },
   };
}