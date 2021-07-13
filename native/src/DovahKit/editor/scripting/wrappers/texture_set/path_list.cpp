#include "path_list.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../wrapper_util.h"

#include "../../../../dovah/forms/TextureSet.h"

namespace {
   using namespace editor_script;
   using cls    = wrappers::texture_set_path_list;
   using form_t = dovah::loaded_forms::TextureSet;

   namespace _getters {
      luastackchange_t backlight(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->textures.backlight.c_str());
         return 1;
      }
      luastackchange_t cubemap(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->textures.cubemap.c_str());
         return 1;
      }
      luastackchange_t detail_map(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         if (!form->is_skin_texture_set())
            return 0;
         lua_pushstring(L, form->textures.glow_map.c_str());
         return 1;
      }
      luastackchange_t diffuse(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->textures.diffuse.c_str());
         return 1;
      }
      luastackchange_t glow_map(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         if (form->is_skin_texture_set())
            return 0;
         lua_pushstring(L, form->textures.glow_map.c_str());
         return 1;
      }
      luastackchange_t environment_mask(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         if (form->is_skin_texture_set())
            return 0;
         lua_pushstring(L, form->textures.environment_mask.c_str());
         return 1;
      }
      luastackchange_t height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->textures.height.c_str());
         return 1;
      }
      luastackchange_t multilayer(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->textures.multilayer.c_str());
         return 1;
      }
      luastackchange_t normal_map(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->textures.normal.c_str());
         return 1;
      }
      luastackchange_t subsurface_tint(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         if (!form->is_skin_texture_set())
            return 0;
         lua_pushstring(L, form->textures.environment_mask.c_str());
         return 1;
      }
   }
   namespace _setters {
      void _require_not_skin_texture_set(lua_State* L, form_t* form, const char* name) {
         if (form->is_skin_texture_set())
            luaL_error(L, "form.textures.%s is only accessible when form.is_skin_texture_set is false", name);
      }
      void _require_skin_texture_set(lua_State* L, form_t* form, const char* name) {
         if (!form->is_skin_texture_set())
            luaL_error(L, "form.textures.%s is only accessible when form.is_skin_texture_set is true", name);
      }

      luastackchange_t backlight(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->textures.backlight = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t cubemap(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->textures.cubemap = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t detail_map(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         _require_skin_texture_set(L, form, "detail_map");
         self.before_edit();
         form->textures.glow_map = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t diffuse(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->textures.diffuse = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t glow_map(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         _require_not_skin_texture_set(L, form, "glow_map");
         self.before_edit();
         form->textures.glow_map = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t environment_mask(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         _require_not_skin_texture_set(L, form, "environment_mask");
         self.before_edit();
         form->textures.environment_mask = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t height(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->textures.height = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t multilayer(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->textures.multilayer = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t normal_map(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->textures.normal = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t subsurface_tint(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         _require_skin_texture_set(L, form, "subsurface_tint");
         self.before_edit();
         form->textures.environment_mask = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "backlight",        &_getters::backlight },
      { "cubemap",          &_getters::cubemap },
      { "detail_map",       &_getters::detail_map },
      { "diffuse",          &_getters::diffuse },
      { "glow_map",         &_getters::glow_map },
      { "environment_mask", &_getters::environment_mask },
      { "height",           &_getters::height },
      { "multilayer",       &_getters::multilayer },
      { "normal_map",       &_getters::normal_map },
      { "subsurface_tint",  &_getters::subsurface_tint },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "backlight",        &_setters::backlight },
      { "cubemap",          &_setters::cubemap },
      { "detail_map",       &_setters::detail_map },
      { "diffuse",          &_setters::diffuse },
      { "glow_map",         &_setters::glow_map },
      { "environment_mask", &_setters::environment_mask },
      { "height",           &_setters::height },
      { "multilayer",       &_setters::multilayer },
      { "normal_map",       &_setters::normal_map },
      { "subsurface_tint",  &_setters::subsurface_tint },
   };
}