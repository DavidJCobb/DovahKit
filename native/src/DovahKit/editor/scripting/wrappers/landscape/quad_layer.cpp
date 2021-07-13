#include "quad_layer.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../wrapper_util.h"

#include "quad_layer.h"

namespace {
   using namespace editor_script;
   using cls    = wrappers::landscape_quad_alpha_layer;
   using form_t = dovah::loaded_forms::Landscape;

   namespace _methods {
      luastackchange_t get_opacity_at_cell_position(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         int isnum;
         int x = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,   2, "integer (x-coordinate) expected");
         luaL_argcheck(L, x > 0,   2, "x-coordinate cannot be zero or negative");
         luaL_argcheck(L, x <= 33, 2, "x-coordinate is outside of the cell");
         int y = lua_tointegerx(L, 3, &isnum);
         luaL_argcheck(L, isnum,   3, "integer (y-coordinate) expected");
         luaL_argcheck(L, y > 0,   3, "y-coordinate cannot be zero or negative");
         luaL_argcheck(L, y <= 33, 3, "y-coordinate is outside of the cell");
         if (!form)
            return 0;
         //
         auto q = self.parts[0].index;
         auto l = self.parts[1].index;
         assert(q >= 0 && q <= 3);
         auto* layer = form->get_alpha_layer(q, l);
         if (!layer)
            return 0;
         int8_t cx = x - 1;
         int8_t cy = y - 1;
         form_t::cell_coords_to_quad_coords(q, cx, cy);
         if (cx < 0 || cy < 0) // coordinates were not in this quad
            return 0;
         lua_pushnumber(L, layer->opacities.at(cx, cy));
         return 1;
      }
   }
   namespace _getters {
      luastackchange_t texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         auto q = self.parts[0].index;
         auto l = self.parts[1].index;
         assert(q >= 0 && q <= 3);
         auto* layer = form->get_alpha_layer(q, l);
         if (!layer)
            return 0;
         return wrap_and_push_form(L, layer->texture);
      }
   }
   namespace _setters {
      luastackchange_t texture(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::land_texture);
         if (!form)
            return 0;
         auto q = self.parts[0].index;
         auto l = self.parts[1].index;
         assert(q >= 0 && q <= 3);
         auto* layer = form->get_alpha_layer(q, l);
         if (!layer)
            return 0;
         //
         self.before_edit();
         layer->texture.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "get_opacity_at_cell_position", &_methods::get_opacity_at_cell_position },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "texture", &_getters::texture },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "texture", &_setters::texture },
   };
}