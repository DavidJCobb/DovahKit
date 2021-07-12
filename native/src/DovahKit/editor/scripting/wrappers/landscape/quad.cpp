#include "quad.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../wrapper_util.h"

namespace {
   using namespace editor_script;
   using cls    = wrappers::landscape_quad;
   using form_t = dovah::loaded_forms::Landscape;

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
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::texture_set);
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
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "default_texture", &_getters::default_texture },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "default_texture", &_setters::default_texture },
   };
}