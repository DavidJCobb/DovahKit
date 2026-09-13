#include "quad.h"
#include "../../../../helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Landscape.h"
#include "quad_layer.h"

namespace {
   using namespace dovahscript;
   using cls    = wrappers::landscape_quad;
   using form_t = dovah::loaded_forms::Landscape;

   namespace _methods {
      int for_each_alpha_layer(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");
         lua_settop(L, 2);
         //
         if (!form)
            return 0;
         auto q = self.last_part().index;
         assert(!self.is_collection);
         assert(q >= 0 && q <= 3);
         //
         auto& list = form->alpha_layers_by_quad[q];
         for (auto& layer : list) {
            lua_pushvalue(L, 2); // function
            wrapper ll = self;
            ll.append_part(wrapper_part_types::landscape_alpha_layer, layer.layer);
            int argc = core::subsystems::userdata::get().push(L, ll, wrappers::landscape_quad_alpha_layer::metatable_key);
            if (argc) {
               lua_call(L, argc, 0);
            } else {
               lua_pop(L, 1); // cancel call; pop function
            }
         }
         return 0;
      }
   }
   namespace _getters {
      int default_texture(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         auto q = self.last_part().index;
         if (q < 0 || q > 3)
            cobb::lua::error(L, "quad wrapper specifies an invalid quad index (%d) (this should never happen)", q);
         return push_native_object(form->default_quad_textures[q]);
      }
   }
   namespace _setters {
      int default_texture(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
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
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "for_each_alpha_layer", &_methods::for_each_alpha_layer },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "default_texture", &_getters::default_texture },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "default_texture", &_setters::default_texture },
   };
}