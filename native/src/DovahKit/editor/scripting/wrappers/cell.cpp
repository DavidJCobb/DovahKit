#include "cell.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../wrapper_util.h"

#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/Cell.h"
#include "../../../dovah/form_stub_helpers.h"
#include "worldspace/grid_bounds_root.h"

//
// MISSING APIS:
//  - CELL/XCLL: Lighting data
//  - CELL/LTMP: Lighting template
//  - CELL/MHDT: Exterior: Max height data
//  - CELL/TVDT: Exterior: Terrain visibility data (occlusion data)
//  - CELL/X...: Extra data (shared with REFR)
//  - CELL/XCLC: Land flags
//  - Access to contained ObjectReferences (but for exterior cells, persistent refs would be in the persistent cell; we'd need special handling for a "friendly" API)
//
#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for cells is incomplete!")
#endif

namespace {
   using namespace editor_script;
   using cls    = wrappers::cell;
   using form_t = dovah::loaded_forms::Cell;

   namespace _methods {
   }
   namespace _getters {
      template<decltype(form_t::cell_flags) flag> luastackchange_t cell_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->cell_flags & flag) != 0);
         return 1;
      }
      template<decltype(form_t::cell_flags) flag> luastackchange_t cell_flag_invert(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->cell_flags & flag) == 0);
         return 1;
      }
      
      luastackchange_t landscape(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         for (auto& pair : stub->inbound) {
            auto& entry = pair.second;
            auto* other = entry.other;
            if (!(entry.flags & dovah::use_info_entry::flag::i_am_parent_of))
               continue;
            if (!other || other->formType != dovah::form_type::land)
               continue;
            return wrap_and_push_form(L, other);
         }
         return 0;
      }
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      luastackchange_t parent_world(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         auto* parent = stub->get_parent_form();
         if (parent && parent->formType == dovah::form_type::worldspace)
            return wrap_and_push_form(L, parent);
         return 0;
      }
      luastackchange_t water_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         if (form->water.height == FLT_MAX) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushnumber(L, form->water.height);
         return 1;
      }
      luastackchange_t water_noise_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->water.noise_texture.c_str());
         return 1;
      }
   }
   namespace _setters {
      template<decltype(form_t::cell_flags) flag> luastackchange_t cell_flag(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->cell_flags, flag, value);
         self.after_edit();
         return 0;
      }
      template<decltype(form_t::cell_flags) flag> luastackchange_t cell_flag_invert(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->cell_flags, flag, !value);
         self.after_edit();
         return 0;
      }
      
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
      luastackchange_t water_height(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         float value = FLT_MAX;
         if (!lua_isnoneornil(L, 2)) {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "number or nil expected");
            value = lua_tonumber(L, 2);
            luaL_argcheck(L, value != FLT_MAX, 2, "FLT_MAX cannot be used as a water height; the game engine uses that value to mean \"no water\"");
            luaL_argcheck(L, value <  FLT_MAX, 2, "water height is a float and cannot exceed FLT_MAX");
         }
         if (!form)
            return 0;
         self.before_edit();
         form->water.height = value;
         self.after_edit();
         return 0;
      }
      luastackchange_t water_noise_texture(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!form)
            return 0;
         self.before_edit();
         form->water.noise_texture = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "allow_fast_travel",   &_getters::cell_flag_invert<form_t::cell_flag::cant_travel_from_here> },
      { "has_lod_water",       &_getters::cell_flag_invert<form_t::cell_flag::no_lod_water> },
      { "has_water",           &_getters::cell_flag<form_t::cell_flag::has_water> }, // NOTE: Game forces this to true on load for exterior cells.
      { "is_hand_changed",     &_getters::cell_flag<form_t::cell_flag::hand_changed> },
      { "is_interior",         &_getters::cell_flag<form_t::cell_flag::interior> },
      { "is_public_area",      &_getters::cell_flag<form_t::cell_flag::public_area> },
      { "landscape",           &_getters::landscape },
      { "name",                &_getters::name },
      { "parent_world",        &_getters::parent_world },
      { "show_sky",            &_getters::cell_flag<form_t::cell_flag::show_sky> },
      { "use_sky_lighting",    &_getters::cell_flag<form_t::cell_flag::use_sky_lighting> },
      { "water_height",        &_getters::water_height },
      { "water_noise_texture", &_getters::water_noise_texture },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "allow_fast_travel",   &_setters::cell_flag_invert<form_t::cell_flag::cant_travel_from_here> },
      { "has_lod_water",       &_setters::cell_flag_invert<form_t::cell_flag::no_lod_water> },
      { "has_water",           &_setters::cell_flag<form_t::cell_flag::has_water> }, // NOTE: Game forces this to true on load for exterior cells.
      { "is_hand_changed",     &_setters::cell_flag<form_t::cell_flag::hand_changed> },
      { "is_interior",         &_setters::cell_flag<form_t::cell_flag::interior> },
      { "is_public_area",      &_setters::cell_flag<form_t::cell_flag::public_area> },
      { "name",                &_setters::name },
      { "show_sky",            &_setters::cell_flag<form_t::cell_flag::show_sky> },
      { "use_sky_lighting",    &_setters::cell_flag<form_t::cell_flag::use_sky_lighting> },
      { "water_height",        &_setters::water_height },
      { "water_noise_texture", &_setters::water_noise_texture },
   };
}