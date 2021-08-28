#include "grid_bounds_root.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Worldspace.h"
#include "grid_bounds.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::worldspace_grid_bounds;
   using wrapped_type = cls::wrapped_type;
}

namespace {
   namespace _getters {
      int min(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::worldspace_bounds_min;
         return core::subsystems::userdata::get().push(L, out, wrappers::worldspace_grid_bounds_extent::metatable_key);
      }
      int max(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::worldspace_bounds_max;
         return core::subsystems::userdata::get().push(L, out, wrappers::worldspace_grid_bounds_extent::metatable_key);
      }
   }
   namespace _setters {
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "min", &_getters::min },
      { "max", &_getters::max },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}