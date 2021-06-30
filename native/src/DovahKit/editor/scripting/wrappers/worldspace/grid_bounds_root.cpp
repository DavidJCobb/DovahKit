#include "grid_bounds_root.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../wrapper_util.h"

#include "grid_bounds.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::worldspace_grid_bounds;
}

namespace {
   namespace _getters {
      luastackchange_t min(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<cls::wrapped_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::worldspace_bounds_min;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::worldspace_grid_bounds_extent::metatable_key);
      }
      luastackchange_t max(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<cls::wrapped_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::worldspace_bounds_max;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::worldspace_grid_bounds_extent::metatable_key);
      }
   }
   namespace _setters {
   }
}
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "min", &_getters::min },
      { "max", &_getters::max },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;
}