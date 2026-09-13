#include "quad_layer.h"
#include "../../../../helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Landscape.h"

namespace {
   using namespace dovahscript;
   using cls    = wrappers::landscape_quad_alpha_layer;
   using wrapped_type = dovah::loaded_forms::Landscape;

   namespace _methods {
      int get_opacity_at_cell_position(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
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
         --x;
         --y;
         lua_pushnumber(L, layer->opacities.item(x, y));
         return 1;
      }
   }
   namespace _getters {
      int texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         auto q = self.parts[0].index;
         auto l = self.parts[1].index;
         assert(q >= 0 && q <= 3);
         auto* layer = form->get_alpha_layer(q, l);
         if (!layer)
            return 0;
         return push_native_object(layer->texture);
      }
   }
   namespace _setters {
      int texture(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
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
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "get_opacity_at_cell_position", &_methods::get_opacity_at_cell_position },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "texture", &_getters::texture },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "texture", &_setters::texture },
   };
}