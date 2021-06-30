#include "grid_bounds.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../wrapper_util.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::worldspace_grid_bounds_extent;
}

namespace {
   namespace _getters {
      luastackchange_t x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<cls::wrapped_t>();
         if (!form)
            return 0;
         switch (self.parts[0].signature) {
            case wrapper_part_types::worldspace_bounds_min:
               lua_pushinteger(L, form->bounds.min.x);
               return 1;
            case wrapper_part_types::worldspace_bounds_max:
               lua_pushinteger(L, form->bounds.max.x);
               return 1;
         }
         luaL_error(L, "internal error: bad wrapper for worldspace grid bounds");
         return 0;
      }
      luastackchange_t y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<cls::wrapped_t>();
         if (!form)
            return 0;
         switch (self.parts[0].signature) {
            case wrapper_part_types::worldspace_bounds_min:
               lua_pushinteger(L, form->bounds.min.y);
               return 1;
            case wrapper_part_types::worldspace_bounds_max:
               lua_pushinteger(L, form->bounds.max.y);
               return 1;
         }
         luaL_error(L, "internal error: bad wrapper for worldspace grid bounds");
         return 0;
      }
   }
   namespace _setters {
      luastackchange_t x(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<cls::wrapped_t>();
         //
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (!form)
            return 0;
         self.before_edit();
         switch (self.parts[0].signature) {
            case wrapper_part_types::worldspace_bounds_min:
               form->bounds.min.x = value;
               break;
            case wrapper_part_types::worldspace_bounds_max:
               form->bounds.max.x = value;
               break;
         }
         self.after_edit();
         return 0;
      }
      luastackchange_t y(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<cls::wrapped_t>();
         //
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (!form)
            return 0;
         self.before_edit();
         switch (self.parts[0].signature) {
            case wrapper_part_types::worldspace_bounds_min:
               form->bounds.min.y = value;
               break;
            case wrapper_part_types::worldspace_bounds_max:
               form->bounds.max.y = value;
               break;
         }
         self.after_edit();
         return 0;
      }
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "x", &_setters::x },
      { "y", &_setters::y },
   };
}