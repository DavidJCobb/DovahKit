#include "worldspace.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/istablelike.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/Worldspace.h"
#include "../../../dovah/form_stubs/helpers/for_each_child_form.h"
#include "../../../dovah/form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "../../../dovah/form_stubs/helpers/get_worldspace_cell_by_grid.h"
#include "worldspace/grid_bounds_root.h"

//
// MISSING APIS:
//  - WRLD/PNAM: Flags to control what features are inherited from the parent worldspace
//  - WRLD/DATA: Worldspace flags
//  - WRLD/MHDT: Max height data (struct)
//  - WRLD/RNAM: Large Refs data (struct) (SSE-only)
//  - WRLD/WCTR: World center coords (struct)
//  - WRLD/ICON: Icon file path (string)
//  - WRLD/MNAM: Map data (struct)
//  - WRLD/ONAM: Map offset data (struct)
//  - WRLD/NAMA: Distant LOD multiplier (float)
//  - Worldspace cloud model (model struct)
//
#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for worldspaces is incomplete.");

namespace {
   using namespace dovahscript;
   using cls          = wrappers::worldspace;
   using wrapped_type = cls::wrapped_type;
   //
   namespace _methods {
      int get_all_cells(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub) {
            lua_createtable(L, 0, 0);
            return 1;
         }
         // The vast majority of references to a WRLD will be its own CELLs, so preallocate the 
         // Lua array to match that size. Beware, however, that Lua only shrinks tables when it 
         // rehashes them (i.e. if they grow too full); if we specify too large a size, the 
         // wasted space will never be freed. Better to specify a size too small; we'll skip 
         // several rehashes and only have to rehash a few times at the end.
         int estimated = stub->inbound.size() / 4 * 3;
         lua_createtable(L, estimated, 0);
         //
         int i   = 0;
         int pos = lua_gettop(L);
         const auto* persistent_cell = dovah::form_stub_helpers::get_worldspace_persistent_cell(*stub);
         dovah::form_stub_helpers::for_each_child_form(*stub, [L, &i, pos, persistent_cell](dovah::form_stub& child) {
            if (&child == persistent_cell)
               return false;
            if (child.form_type != dovah::form_type::cell)
               return false;
            int wcount = push_native_object(&child);
            while (wcount--)
               lua_rawseti(L, pos, ++i);
            return false;
         });
         assert(lua_gettop(L) == pos);
         //
         return 1;
      }
      int get_cell_from_grid(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         int isnum;
         int gx = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "grid-x (integer) expected");
         int gy = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "grid-y (integer) expected");
         if (!stub)
            return 0;
         auto* cell = dovah::form_stub_helpers::get_worldspace_cell_by_grid(*stub, gx, gy);
         return push_native_object(cell);
      }
   }
   namespace _getters {
      int bounds(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::worldspace_bounds);
         return core::subsystems::userdata::get().push(L, out, wrappers::worldspace_grid_bounds::metatable_key);
      }
      int climate(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->climate);
      }
      int default_land_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->land_data.default_land_height);
         return 1;
      }
      int default_water_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->land_data.default_water_height);
         return 1;
      }
      int encounter_zone(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->encounter_zone);
      }
      int hd_lod_diffuse_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->hd_lod_diffuse_texture.c_str());
         return 1;
      }
      int hd_lod_normal_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->hd_lod_normal_texture.c_str());
         return 1;
      }
      int lighting_template(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->lighting_template);
      }
      int location(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->location);
      }
      int lod_water_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->lod_water_height);
         return 1;
      }
      int music_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->music);
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      int parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->parent.form.get_form_stub());
      }
      int tree_canopy_shadow(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->tree_canopy_shadow.c_str());
         return 1;
      }
      int water_environment_map(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->water_environment_map.c_str());
         return 1;
      }
      int water_noise_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->water_noise_texture.c_str());
         return 1;
      }
      int water_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->water_type);
      }
      int water_type_lod(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->water_type_lod);
      }
   }
   namespace _setters {
      int climate(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::climate);
         if (!form)
            return 0;
         self.before_edit();
         form->climate.set(*form, value);
         self.after_edit();
         return 0;
      }
      int default_land_height(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->land_data.default_land_height = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int default_water_height(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->land_data.default_water_height = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int encounter_zone(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::encounter_zone);
         if (!form)
            return 0;
         self.before_edit();
         form->encounter_zone.set(*form, value);
         self.after_edit();
         return 0;
      }
      int hd_lod_diffuse_texture(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!form)
            return 0;
         self.before_edit();
         form->hd_lod_diffuse_texture = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int hd_lod_normal_texture(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!form)
            return 0;
         self.before_edit();
         form->hd_lod_normal_texture = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int lighting_template(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::lighting_template);
         if (!form)
            return 0;
         self.before_edit();
         form->lighting_template.set(*form, value);
         self.after_edit();
         return 0;
      }
      int location(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::location);
         if (!form)
            return 0;
         self.before_edit();
         form->location.set(*form, value);
         self.after_edit();
         return 0;
      }
      int lod_water_height(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->lod_water_height = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int music_type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::music_type);
         if (!form)
            return 0;
         self.before_edit();
         form->music.set(*form, value);
         self.after_edit();
         return 0;
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 1;
      }
      int parent(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::worldspace);
         if (!form)
            return 0;
         self.before_edit();
         form->parent.form.set(*form, value);
         self.after_edit();
         return 0;
      }
      int tree_canopy_shadow(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!form)
            return 0;
         self.before_edit();
         form->tree_canopy_shadow = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int water_environment_map(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!form)
            return 0;
         self.before_edit();
         form->water_environment_map = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int water_noise_texture(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!form)
            return 0;
         self.before_edit();
         form->water_noise_texture = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int water_type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::water_type);
         if (!form)
            return 0;
         self.before_edit();
         form->water_type.set(*form, value);
         self.after_edit();
         return 0;
      }
      int water_type_lod(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
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

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "get_all_cells",      &_methods::get_all_cells },      // Perf hit from creating an array of wrappers for all cells. Tamriel has 11186 cells in it!
      { "get_cell_from_grid", &_methods::get_cell_from_grid }, // Perf hit from having to loop through the worldspace's child cells for each query.
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "bounds",                 &_getters::bounds }, // NAM0 and NAM9
      { "climate",                &_getters::climate },
      { "default_land_height",    &_getters::default_land_height },
      { "default_water_height",   &_getters::default_water_height },
      { "encounter_zone",         &_getters::encounter_zone },
      { "hd_lod_diffuse_texture", &_getters::hd_lod_diffuse_texture },
      { "hd_lod_normal_texture",  &_getters::hd_lod_normal_texture },
      { "lighting_template",      &_getters::lighting_template },
      { "location",               &_getters::location },
      { "lod_water_height",       &_getters::lod_water_height },
      { "music_type",             &_getters::music_type },
      { "name",                   &_getters::name },
      { "parent",                 &_getters::parent },
      { "tree_canopy_shadow",     &_getters::tree_canopy_shadow },
      { "water_environment_map",  &_getters::water_environment_map },
      { "water_noise_texture",    &_getters::water_noise_texture },
      { "water_type",             &_getters::water_type },
      { "water_type_lod",         &_getters::water_type_lod },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "climate",                &_setters::climate },
      { "default_land_height",    &_setters::default_land_height },
      { "default_water_height",   &_setters::default_water_height },
      { "encounter_zone",         &_setters::encounter_zone },
      { "hd_lod_diffuse_texture", &_setters::hd_lod_diffuse_texture },
      { "hd_lod_normal_texture",  &_setters::hd_lod_normal_texture },
      { "lighting_template",      &_setters::lighting_template },
      { "location",               &_setters::location },
      { "lod_water_height",       &_setters::lod_water_height },
      { "music_type",             &_setters::music_type },
      { "name",                   &_setters::name },
      { "parent",                 &_setters::parent },
      { "tree_canopy_shadow",     &_setters::tree_canopy_shadow },
      { "water_environment_map",  &_setters::water_environment_map },
      { "water_noise_texture",    &_setters::water_noise_texture },
      { "water_type",             &_setters::water_type },
      { "water_type_lod",         &_setters::water_type_lod },
   };
}