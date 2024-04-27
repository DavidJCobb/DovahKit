#include "position.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "dovah/exceptions/object_reference_move_failed.h"
#include "dovah/forms/ObjectReference.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::objectreference_position;
   using wrapped_type = dovah::loaded_forms::ObjectReference;

   namespace _helpers {
      void set_position(lua_State* L, wrapper& self, wrapped_type* form, cobb::vector3<float> position) {
         using exception  = dovah::exceptions::object_reference_move_failed;
         using error_code = exception::error_code;

         self.before_edit();
         try {
            form->set_position(position);
         } catch (const exception& ex) {
            self.after_edit(); // TODO: Add a way to signal a failed edit (i.e. so we don't mark the form as edited). TODO: RAII (in lieu of no "finally" blocks).
            switch (ex.code) {
               case error_code::reference_is_orphaned:
                  cobb::lua::error(L, "this reference has no parent cell (PlayerRef?), so its position cannot safely be set");
               case error_code::desired_position_is_outside_of_desired_cell: // shouldn't happen, as we're not requesting a specific cell
                  break;
               case error_code::failed_to_create_destination_cell:
                  cobb::lua::error(L, "the desired position lies outside of any existing cells, and DovahKit was unable to create a new cell");
               case error_code::reference_is_hardcoded:
                  cobb::lua::error(L, "hardcoded references cannot safely be reparented");
               case error_code::operation_not_allowed_on_form_working_copy: // shouldn't happen, as Lua should never be operating on working copies
                  break;
            }
            cobb::lua::error(L, "an internal error occurred while trying to set the form's position");
            return;
         }
         self.after_edit();
      }
   }

   namespace _methods {
      int set_xyz(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number (x) expected");
         cobb::lua::argcheck(L, lua_isnumber(L, 3), 3, "number (y) expected");
         cobb::lua::argcheck(L, lua_isnumber(L, 4), 4, "number (z) expected");
         float x = lua_tonumber(L, 2);
         float y = lua_tonumber(L, 3);
         float z = lua_tonumber(L, 4);
         _helpers::set_position(L, self, form, { x, y, z });
         return 0;
      }
   }
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
      int x(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         if (!form)
            return 0;
         auto pos = form->position;
         pos.x = lua_tonumber(L, 2);
         _helpers::set_position(L, self, form, pos);
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
         auto pos = form->position;
         pos.y = lua_tonumber(L, 2);
         _helpers::set_position(L, self, form, pos);
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
         auto pos = form->position;
         pos.z = lua_tonumber(L, 2);
         _helpers::set_position(L, self, form, pos);
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "set_xyz", &_methods::set_xyz },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
      { "z", &_getters::z },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "x", &_setters::x },
      { "y", &_setters::y },
      { "z", &_setters::z },
   };
}