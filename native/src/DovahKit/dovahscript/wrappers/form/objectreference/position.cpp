#include "position.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/ObjectReference.h"

#include "../../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for objectreference_position is incomplete.");
//
// - Setters
// 
//    - We need code in the backend to modify a reference's position, reparenting it (and creating 
//      a new cell in the containing world if necessary). This code needs to be able to fail (i.e. 
//      if no form IDs are available for a new cell).
// 
//       - We might benefit from being able to test whether a cell is "untouched," i.e. whether 
//         it's completely default. If a cell is created solely as a result of REFRs being moved 
//         inside, and those REFRs are then moved away, without any further changes to the cell, 
//         then perhaps we should delete the cell.
// 
//         Consider this an "extra," however. It's not essential for a minimum viable implementation.
//

namespace {
   using namespace dovahscript;
   using cls          = wrappers::objectreference_position;
   using wrapped_type = dovah::loaded_forms::ObjectReference;

   namespace _getters {
      int x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->position.x);
         return 1;
      }
      int y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->position.y);
         return 1;
      }
      int z(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->position.z);
         return 1;
      }
   }
   namespace _setters {
      /*//
      int x(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->position.x = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int y(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->position.y = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int z(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         self.before_edit();
         form->position.z = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      //*/
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
      { "z", &_getters::z },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      /*//
      { "x", &_setters::x },
      { "y", &_setters::y },
      { "z", &_setters::z },
      //*/
   };
}