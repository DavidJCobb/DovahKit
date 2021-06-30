#include "worldspace.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../wrapper_util.h"

#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/Worldspace.h"
#include "../../../dovah/form_stub_helpers.h"

namespace {
   using namespace editor_script;
   using cls    = wrappers::worldspace;
   using form_t = dovah::loaded_forms::Worldspace;
   //
   namespace _methods {
      luastackchange_t get_cell_from_grid(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         int isnum;
         int gx = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "grid-x (integer) expected");
         int gy = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "grid-y (integer) expected");
         if (!stub)
            return 0;
         auto* cell = dovah::form_stub_helpers::get_worldspace_cell_by_grid(stub, gx, gy);
         return wrap_and_push_form(L, cell);
      }
   }
   namespace _getters {
      luastackchange_t climate(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->climate);
      }
      luastackchange_t default_land_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->land_data.default_land_height);
         return 1;
      }
      luastackchange_t default_water_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->land_data.default_land_height);
         return 1;
      }
      luastackchange_t encounter_zone(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->encounter_zone);
      }
      luastackchange_t lighting_template(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->lighting_template);
      }
      luastackchange_t location(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->location);
      }
      luastackchange_t lod_water_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->lod_water_height);
         return 1;
      }
      luastackchange_t music_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->music);
      }
      luastackchange_t parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->parent.form.get_form_stub());
      }
      luastackchange_t water_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->water_type);
      }
      luastackchange_t water_type_lod(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->water_type_lod);
      }
   }
   namespace _setters {
      luastackchange_t climate(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::climate);
         if (!form)
            return 0;
         self.before_edit();
         form->climate.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t default_land_height(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->land_data.default_land_height = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t default_water_height(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->land_data.default_water_height = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t encounter_zone(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::encounter_zone);
         if (!form)
            return 0;
         self.before_edit();
         form->encounter_zone.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t lighting_template(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::lighting_template);
         if (!form)
            return 0;
         self.before_edit();
         form->lighting_template.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t location(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::location);
         if (!form)
            return 0;
         self.before_edit();
         form->location.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t lod_water_height(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->lod_water_height = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t music_type(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::music_type);
         if (!form)
            return 0;
         self.before_edit();
         form->music.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t parent(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::worldspace);
         if (!form)
            return 0;
         self.before_edit();
         form->parent.form.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t water_type(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::water_type);
         if (!form)
            return 0;
         self.before_edit();
         form->water_type.set(*form, value);
         self.after_edit();
         return 0;
      }
      luastackchange_t water_type_lod(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::water_type);
         if (!form)
            return 0;
         self.before_edit();
         form->water_type_lod.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "get_cell_from_grid", &_methods::get_cell_from_grid },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "climate",              &_getters::climate },
      { "default_land_height",  &_getters::default_land_height },
      { "default_water_height", &_getters::default_water_height },
      { "encounter_zone",       &_getters::encounter_zone },
      { "lighting_template",    &_getters::lighting_template },
      { "location",             &_getters::location },
      { "lod_water_height",     &_getters::lod_water_height },
      { "music_type",           &_getters::music_type },
      { "parent",               &_getters::parent },
      { "water_type",           &_getters::water_type },
      { "water_type_lod",       &_getters::water_type_lod },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "climate",              &_setters::climate },
      { "default_land_height",  &_setters::default_land_height },
      { "default_water_height", &_setters::default_water_height },
      { "encounter_zone",       &_setters::encounter_zone },
      { "lighting_template",    &_setters::lighting_template },
      { "location",             &_setters::location },
      { "lod_water_height",     &_setters::lod_water_height },
      { "music_type",           &_setters::music_type },
      { "parent",               &_setters::parent },
      { "water_type",           &_setters::water_type },
      { "water_type_lod",       &_setters::water_type_lod },
   };
}