#include "grid_bounds.h"
#include "../../../../helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Worldspace.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::worldspace_grid_bounds_extent;
   using wrapped_type = cls::wrapped_type;
}

namespace {
   namespace _getters {
      int x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
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
         cobb::lua::error(L, "internal error: bad wrapper for worldspace grid bounds");
      }
      int y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
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
         cobb::lua::error(L, "internal error: bad wrapper for worldspace grid bounds");
      }
   }
   namespace _setters {
      int x(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
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
      int y(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
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
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "x", &_setters::x },
      { "y", &_setters::y },
   };
}