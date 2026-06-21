#include "placed_hazard.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Form.h"
#include "dovah/forms/PlacedHazard.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::placed_hazard;
   using wrapped_type = dovah::loaded_forms::ObjectReference;

   namespace _setters {
      int base_form(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2);
         cobb::lua::argcheck(L, value != nullptr, 2, "form expected");
         if (!dovah::form_type_is_base_form(value->form_type)) {
            cobb::lua::argerror(L, 2, "the provided form is not a base form");
         }
         if (value->form_type != dovah::form_type::hazard) {
            cobb::lua::argerror(L, 2, "the provided form is not a Hazard; you cannot change a Placed Hazard's base form to a non-Hazard");
         }
         if (!form)
            return 0;
         self.before_edit();
         form->base_form.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = no_functions;
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "base_form", &_setters::base_form },
   };
}