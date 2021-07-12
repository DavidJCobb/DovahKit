#include "landscape.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../wrapper_util.h"

#include "../classes.h"
#include "../../../dovah/forms/Landscape.h"
#include "../../../dovah/forms/Worldspace.h"
#include "../../../dovah/form_stub_helpers.h"

#include "../../../helpers/lua/istablelike.h"

#include "landscape/quad_list.h"

//
// MISSING APIS:
//  - LAND/DATA:      Flags
//  - LAND/VHGT:      Vertex heights
//     - Method to modify heights
//  - LAND/VNML:      Vertex normals
//  - LAND/VCLR:      Vertex colors   // IMPLEMENTED BUT NOT TESTED
//  - LAND/BTXT:      Base textures
//  - LAND/ATXT+VTXT: Layer data
//  - LAND/VTEX:      Texture list(?)
//
#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for landscapes is incomplete!")
#endif

namespace {
   using namespace editor_script;
   using cls    = wrappers::landscape;
   using form_t = dovah::loaded_forms::Landscape;

   namespace _methods {
      luastackchange_t get_color_at(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer (x-coordinate) expected");
         luaL_argcheck(L, x > 0, 2, "x-coordinate cannot be less than 1");
         luaL_argcheck(L, x <= form_t::vertices_per_side, 2, "x-coordinate cannot exceed 33");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "integer (y-coordinate) expected");
         luaL_argcheck(L, y > 0, 3, "y-coordinate cannot be less than 1");
         luaL_argcheck(L, y <= form_t::vertices_per_side, 2, "y-coordinate cannot exceed 33");
         if (!form)
            return 0;
         --x;
         --y;
         //
         const auto& color = form->heightmap.colors.at(x, y);
         lua_createtable(L, 3, 3);
         //
         lua_pushinteger(L, color.r);
         lua_pushvalue  (L, -1);
         lua_seti    (L, -3, 1);
         lua_setfield(L, -2, "r");
         //
         lua_pushinteger(L, color.g);
         lua_pushvalue  (L, -1);
         lua_seti    (L, -3, 2);
         lua_setfield(L, -2, "g");
         //
         lua_pushinteger(L, color.b);
         lua_pushvalue  (L, -1);
         lua_seti    (L, -3, 3);
         lua_setfield(L, -2, "b");
         //
         return 1;
      }
      luastackchange_t get_height_at(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer (x-coordinate) expected");
         luaL_argcheck(L, x > 0, 2, "x-coordinate cannot be less than 1");
         luaL_argcheck(L, x <= form_t::vertices_per_side, 2, "x-coordinate cannot exceed 33");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "integer (y-coordinate) expected");
         luaL_argcheck(L, y > 0, 3, "y-coordinate cannot be less than 1");
         luaL_argcheck(L, y <= form_t::vertices_per_side, 2, "y-coordinate cannot exceed 33");
         if (!form)
            return 0;
         --x;
         --y;
         lua_pushnumber(L, form->heightmap.heights.at(x, y));
         return 1;
      }
      luastackchange_t get_maximum_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         float max = std::numeric_limits<float>::lowest(); // ::min() isn't actually the minimum for floating-point types
         for (auto f : form->heightmap.heights.list)
            if (f > max)
               max = f;
         lua_pushnumber(L, max);
         return 1;
      }
      luastackchange_t get_minimum_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         float min = std::numeric_limits<float>::max();
         for (auto f : form->heightmap.heights.list)
            if (f < min)
               min = f;
         lua_pushnumber(L, min);
         return 1;
      }
      luastackchange_t set_color_at(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         lua_settop(L, 4);
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer (x-coordinate) expected");
         luaL_argcheck(L, x > 0, 2, "x-coordinate cannot be less than 1");
         luaL_argcheck(L, x <= form_t::vertices_per_side, 2, "x-coordinate cannot exceed 33");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum, 3, "integer (y-coordinate) expected");
         luaL_argcheck(L, y > 0, 3, "y-coordinate cannot be less than 1");
         luaL_argcheck(L, y <= form_t::vertices_per_side, 2, "y-coordinate cannot exceed 33");
         //
         int r, g, b;
         if (lua_isnoneornil(L, 4)) {
            r = 255;
            g = 255;
            b = 255;
         } else {
            luaL_argcheck(L, cobb::lua::istablelike(L, 4), 4, "table or nil expected");
            //
            lua_getfield(L, 4, "r");
            r = lua_tointegerx(L, 5, &isnum);
            lua_pop(L, 1);
            if (!isnum) {
               lua_geti(L, 4, 1);
               r = lua_tointegerx(L, 5, &isnum);
               lua_pop(L, 1);
               luaL_argcheck(L, isnum, 4, "table did not have an `r` or `1` field");
            }
            luaL_argcheck(L, r >=   0, 4, "the specified color's red component was negative");
            luaL_argcheck(L, r <= 255, 4, "the specified color's red component cannot be above 255");
            //
            lua_getfield(L, 4, "g");
            g = lua_tointegerx(L, 5, &isnum);
            lua_pop(L, 1);
            if (!isnum) {
               lua_geti(L, 4, 1);
               g = lua_tointegerx(L, 5, &isnum);
               lua_pop(L, 1);
               luaL_argcheck(L, isnum, 4, "table did not have an `g` or `2` field");
            }
            luaL_argcheck(L, g >=   0, 4, "the specified color's green component was negative");
            luaL_argcheck(L, g <= 255, 4, "the specified color's green component cannot be above 255");
            //
            lua_getfield(L, 4, "r");
            b = lua_tointegerx(L, 5, &isnum);
            lua_pop(L, 1);
            if (!isnum) {
               lua_geti(L, 4, 1);
               b = lua_tointegerx(L, 5, &isnum);
               lua_pop(L, 1);
               luaL_argcheck(L, isnum, 4, "table did not have an `b` or `3` field");
            }
            luaL_argcheck(L, b >=   0, 4, "the specified color's blue component was negative");
            luaL_argcheck(L, b <= 255, 4, "the specified color's blue component cannot be above 255");
         }
         //
         if (!form)
            return 0;
         --x;
         --y;
         //
         self.before_edit();
         auto& color = form->heightmap.colors.at(x, y);
         color.r = r;
         color.g = g;
         color.b = b;
         self.after_edit();
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t enable_paint_layers(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, form->land_flags & form_t::land_flag::has_layers);
         return 1;
      }
      luastackchange_t enable_vertex_colors(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, form->land_flags & form_t::land_flag::has_colors);
         return 1;
      }
      luastackchange_t enable_vertex_heights(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, form->land_flags & form_t::land_flag::has_heightmap);
         return 1;
      }
      luastackchange_t parent_cell(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         auto* parent = stub->get_parent_form();
         if (parent && parent->formType == dovah::form_type::cell)
            return wrap_and_push_form(L, parent);
         return 0;
      }
      luastackchange_t quads(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::landscape_quad);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::landscape_quad_list::metatable_key);
      }
   }
   namespace _setters {
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "get_color_at",       &_methods::get_color_at },
      { "get_height_at",      &_methods::get_height_at },
      { "get_maximum_height", &_methods::get_maximum_height },
      { "get_minimum_height", &_methods::get_minimum_height },
      { "set_color_at",       &_methods::set_color_at },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "enable_paint_layers",   &_getters::enable_paint_layers },
      { "enable_vertex_colors",  &_getters::enable_vertex_colors },
      { "enable_vertex_heights", &_getters::enable_vertex_heights },
      { "parent_cell",           &_getters::parent_cell },
      { "quads",                 &_getters::quads },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };
}