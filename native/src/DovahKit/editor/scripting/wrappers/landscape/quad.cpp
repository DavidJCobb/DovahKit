#include "quad.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../wrapper_util.h"

#include "quad_layer.h"

namespace {
   using namespace editor_script;
   using cls    = wrappers::landscape_quad;
   using form_t = dovah::loaded_forms::Landscape;

   namespace _methods {
      luastackchange_t for_each_alpha_layer(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");
         lua_settop(L, 2);
         //
         if (!form)
            return 0;
         auto q = self.last_part().index;
         assert(q >= 0 && q <= 3);
         //
         auto& list = form->alpha_layers_by_quad[q];
         for (auto& layer : list) {
            lua_pushvalue(L, 2); // function
            wrapper ll = self;
            ll.append_part(wrapper_part_types::landscape_alpha_layer, layer.layer);
            int argc = DovahKitScriptVMUserdataInterface::get().push(L, ll, wrappers::landscape_quad_alpha_layer::metatable_key);
            lua_call(L, argc, 0);
         }
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t default_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         auto q = self.last_part().index;
         assert(q >= 0 && q <= 3);
         return wrap_and_push_form(L, form->default_quad_textures[q]);
      }
   }
   namespace _setters {
      luastackchange_t default_texture(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::land_texture);
         if (!form)
            return 0;
         auto q = self.last_part().index;
         assert(q >= 0 && q <= 3);
         //
         self.before_edit();
         form->default_quad_textures[q].set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "for_each_alpha_layer", &_methods::for_each_alpha_layer },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "default_texture", &_getters::default_texture },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "default_texture", &_setters::default_texture },
   };
}