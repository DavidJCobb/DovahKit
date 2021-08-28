#include "formlist.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"

#include "../../wrapper.h"
#include "../../core/classes.h"
#include "../../core/collections.h"
#include "../../push_native_object.h"
#include "../../lua_libraries/form_types.h"

#include "../../../dovah/forms/FormList.h"
#include "formlist/collection_entries.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::formlist;
   using wrapped_type = dovah::loaded_forms::FormList;

   namespace _getters {
      int entries(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::formlist_entries);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::formlist_entries.registry_key);
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "entries", &_getters::entries },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, collections::formlist_entries);
   }
}
#pragma endregion