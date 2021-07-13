#include "texture_set.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/TextureSet.h"

#include "texture_set/path_list.h"

//
// MISSING APIS:
//  - TXST/DODT: Decal Object Data
//  - TXST/OBND: Object Bounds
//
#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for texture_sets is incomplete!")
#endif

namespace {
   using namespace editor_script;
   using cls    = wrappers::texture_set;
   using form_t = dovah::loaded_forms::TextureSet;
   
   namespace _methods {
   }
   namespace _getters {
      luastackchange_t has_model_space_normals(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->texture_flags & form_t::texture_set_flag::has_model_space_normals));
         return 1;
      }
      luastackchange_t is_skin_texture_set(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->texture_flags & form_t::texture_set_flag::is_skin_textures));
         return 1;
      }
      luastackchange_t textures(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::texture_set_paths);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::texture_set_path_list::metatable_key);
      }
   }
   namespace _setters {
      luastackchange_t has_model_space_normals(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->texture_flags, form_t::texture_set_flag::has_model_space_normals, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      luastackchange_t is_skin_texture_set(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         auto* form = self.get_loaded_form_data<form_t>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->texture_flags, form_t::texture_set_flag::is_skin_textures, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "has_model_space_normals", &_getters::has_model_space_normals },
      { "is_skin_texture_set",     &_getters::is_skin_texture_set },
      { "textures",                &_getters::textures },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "has_model_space_normals", &_setters::has_model_space_normals },
      { "is_skin_texture_set",     &_setters::is_skin_texture_set },
   };
}