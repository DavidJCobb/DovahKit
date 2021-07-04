#include "grid_coords.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../wrapper_util.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::cell_grid_coords;
}

namespace {
   namespace _getters {
      luastackchange_t x(lua_State* L) {
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
      luastackchange_t y(lua_State* L) {
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
namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "x", &_getters::x },
      { "y", &_getters::y },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;
}