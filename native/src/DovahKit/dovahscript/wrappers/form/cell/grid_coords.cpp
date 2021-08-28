#include "grid_coords.h"
#include "../../../../helpers/lua/error.h"
#include "../../../wrapper.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::cell_grid_coords;
}

namespace {
   namespace _getters {
      int x(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         int32_t x;
         int32_t y;
         if (stub->get_grid_coordinates(x, y)) {
            lua_pushinteger(L, x);
            return 1;
         }
         return 0;
      }
      int y(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub)
            return 0;
         int32_t x;
         int32_t y;
         if (stub->get_grid_coordinates(x, y)) {
            lua_pushinteger(L, y);
            return 1;
         }
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
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}