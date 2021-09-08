#include "texture_set.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/TextureSet.h"
#include "texture_set/path_list.h"

//
// MISSING APIS:
//  - TXST/DODT: Decal Object Data
//  - TXST/OBND: Object Bounds
//
#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for texture_sets is incomplete.");

namespace {
   using namespace dovahscript;
   using cls          = wrappers::texture_set;
   using wrapped_type = cls::wrapped_type;
   
   namespace _methods {
   }
   namespace _getters {
      int has_model_space_normals(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->texture_flags & wrapped_type::texture_set_flag::has_model_space_normals));
         return 1;
      }
      int is_skin_texture_set(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->texture_flags & wrapped_type::texture_set_flag::is_skin_textures));
         return 1;
      }
      int textures(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::texture_set_paths);
         return core::subsystems::userdata::get().push(L, out, wrappers::texture_set_path_list::metatable_key);
      }
   }
   namespace _setters {
      int has_model_space_normals(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->texture_flags, wrapped_type::texture_set_flag::has_model_space_normals, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int is_skin_texture_set(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->texture_flags, wrapped_type::texture_set_flag::is_skin_textures, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "has_model_space_normals", &_getters::has_model_space_normals },
      { "is_skin_texture_set",     &_getters::is_skin_texture_set },
      { "textures",                &_getters::textures },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "has_model_space_normals", &_setters::has_model_space_normals },
      { "is_skin_texture_set",     &_setters::is_skin_texture_set },
   };
}