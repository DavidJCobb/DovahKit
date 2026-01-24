#include "cell.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"

#include "../../wrapper.h"
#include "../../core/classes.h"
#include "../../core/collections.h"
#include "../../push_native_object.h"
#include "../../lua_libraries/form_types.h"

#include "../../../dovah/forms/Cell.h"
#include "../../../dovah/form_stubs/helpers/for_each_child_form.h"
#include "../../../dovah/form_stubs/helpers/get_cell_landscape.h"
#include "cell/grid_coords.h"

//
// MISSING APIS:
//  - CELL/XCLL: Lighting data
//  - CELL/LTMP: Lighting template
//  - CELL/MHDT: Exterior: Max height data
//  - CELL/TVDT: Exterior: Terrain visibility data (occlusion data)
//  - CELL/X...: Extra data (shared with REFR)
//  - CELL/XCLC: Land flags
//
#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for cells is incomplete.");

namespace {
   using namespace dovahscript;
   using cls          = wrappers::cell;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int get_all_persistent_refs(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         lua_settop(L, 1);
         lua_createtable(L, 0, 0);
         int i = 1;
         dovah::form_stub_helpers::for_each_child_form(*stub, [L, &i](dovah::form_stub& child) {
            if (!dovah::form_type_is_reference(child.form_type))
               return false;
            if (!child.test_record_flags(dovah::tes_file_record_header::flag::persistent))
               return false;
            int argcount = push_native_object(&child);
            while (argcount--)
               lua_rawseti(L, 2, i++);
            return false;
         });
         return 1;
      }
      int get_all_children(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         lua_settop(L, 1);
         lua_createtable(L, stub->inbound.size(), 0);
         int i = 1;
         dovah::form_stub_helpers::for_each_child_form(*stub, [L, &i](dovah::form_stub& child) {
            int argcount = push_native_object(&child);
            while (argcount--)
               lua_rawseti(L, 2, i++);
            return false;
         });
         return 1;
      }
      int get_all_refs(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         lua_settop(L, 1);
         lua_createtable(L, stub->inbound.size() / 2, 0);
         int i = 1;
         dovah::form_stub_helpers::for_each_child_form(*stub, [L, &i](dovah::form_stub& child) {
            if (!dovah::form_type_is_reference(child.form_type))
               return false;
            int argcount = push_native_object(&child);
            while (argcount--)
               lua_rawseti(L, 2, i++);
            return false;
         });
         return 1;
      }
   }
   namespace _getters {
      template<decltype(wrapped_type::cell_flags) flag> int cell_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->cell_flags & flag) != 0);
         return 1;
      }
      template<decltype(wrapped_type::cell_flags) flag> int cell_flag_invert(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->cell_flags & flag) == 0);
         return 1;
      }
      
      int grid_coords(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         int32_t x;
         int32_t y;
         if (stub->get_grid_coordinates(x, y)) {
            wrapper out = self;
            out.append_part(wrapper_part_types::cell_grid_coords);
            return core::subsystems::userdata::get().push(L, out, wrappers::cell_grid_coords::metatable_key);
         }
         return 0;
      }
      int landscape(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         return push_native_object(dovah::form_stub_helpers::get_cell_landscape(*stub));
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      int parent_world(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         auto* parent = stub->get_parent_form();
         if (parent && parent->form_type == dovah::form_type::worldspace)
            return push_native_object(parent);
         return 0;
      }
      int water_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         if (form->water.height >= wrapped_type::inherit_water_height) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushnumber(L, form->water.height);
         return 1;
      }
      int water_noise_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->water.noise_texture.c_str());
         return 1;
      }
   }
   namespace _setters {
      template<decltype(wrapped_type::cell_flags) flag> int cell_flag(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->cell_flags, flag, value);
         self.after_edit();
         return 0;
      }
      template<decltype(wrapped_type::cell_flags) flag> int cell_flag_invert(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->cell_flags, flag, !value);
         self.after_edit();
         return 0;
      }
      
      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
      int water_height(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         float value = wrapped_type::absent_water_height;
         if (!lua_isnoneornil(L, 2)) {
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "number or nil expected");
            value = lua_tonumber(L, 2);
            luaL_argcheck(L, value <= wrapped_type::inherit_water_height, 2, "the game caps water height to below 2147483648; pass a lower value, or pass nil to inherit the parent worldspace's height");
         }
         if (!form)
            return 0;
         self.before_edit();
         form->water.height = value;
         self.after_edit();
         return 0;
      }
      int water_noise_texture(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
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

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "get_all_children",        &_methods::get_all_children },
      { "get_all_persistent_refs", &_methods::get_all_persistent_refs },
      { "get_all_refs",            &_methods::get_all_refs },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "allow_fast_travel",   &_getters::cell_flag_invert<wrapped_type::cell_flag::cant_travel_from_here> },
      { "grid_coords",         &_getters::grid_coords },
      { "has_lod_water",       &_getters::cell_flag_invert<wrapped_type::cell_flag::no_lod_water> },
      { "has_water",           &_getters::cell_flag<wrapped_type::cell_flag::has_water> }, // NOTE: Game forces this to true on load for exterior cells.
      { "is_hand_changed",     &_getters::cell_flag<wrapped_type::cell_flag::hand_changed> },
      { "is_interior",         &_getters::cell_flag<wrapped_type::cell_flag::interior> },
      { "is_public_area",      &_getters::cell_flag<wrapped_type::cell_flag::public_area> },
      { "landscape",           &_getters::landscape },
      { "name",                &_getters::name },
      { "parent_world",        &_getters::parent_world },
      { "show_sky",            &_getters::cell_flag<wrapped_type::cell_flag::show_sky> },
      { "use_sky_lighting",    &_getters::cell_flag<wrapped_type::cell_flag::use_sky_lighting> },
      { "water_height",        &_getters::water_height },
      { "water_noise_texture", &_getters::water_noise_texture },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "allow_fast_travel",   &_setters::cell_flag_invert<wrapped_type::cell_flag::cant_travel_from_here> },
      { "has_lod_water",       &_setters::cell_flag_invert<wrapped_type::cell_flag::no_lod_water> },
      { "has_water",           &_setters::cell_flag<wrapped_type::cell_flag::has_water> }, // NOTE: Game forces this to true on load for exterior cells.
      { "is_hand_changed",     &_setters::cell_flag<wrapped_type::cell_flag::hand_changed> },
      { "is_interior",         &_setters::cell_flag<wrapped_type::cell_flag::interior> },
      { "is_public_area",      &_setters::cell_flag<wrapped_type::cell_flag::public_area> },
      { "name",                &_setters::name },
      { "show_sky",            &_setters::cell_flag<wrapped_type::cell_flag::show_sky> },
      { "use_sky_lighting",    &_setters::cell_flag<wrapped_type::cell_flag::use_sky_lighting> },
      { "water_height",        &_setters::water_height },
      { "water_noise_texture", &_setters::water_noise_texture },
   };
}