#include "landscape.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../wrapper_util.h"

#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/Landscape.h"
#include "../../../dovah/form_stub_helpers.h"

//
// MISSING APIS:
//  - LAND/DATA:      Flags
//  - LAND/VHGT:      Vertex heights
//     - Method to modify heights
//  - LAND/VNML:      Vertex normals
//  - LAND/VCLR:      Vertex colors
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
         float max = std::numeric_limits<float>::min();
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
   }
   namespace _getters {
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
   }
   namespace _setters {
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "get_height_at",      &_methods::get_height_at },
      { "get_maximum_height", &_methods::get_maximum_height },
      { "get_minimum_height", &_methods::get_minimum_height },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "parent_cell", &_getters::parent_cell },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };
}