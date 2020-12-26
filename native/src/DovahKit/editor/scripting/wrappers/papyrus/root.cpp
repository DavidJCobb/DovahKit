#include "root.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../../helpers/lua/metamethod_names.h"

namespace {
   using namespace editor_script;

   #pragma region __pairs iterator
   namespace __pairs {
      constexpr char* metatable_key = "pairs_iterator<dovah.classes.papyrus_root>";
      //
      namespace {
         bool _should_skip_name(lua_State* L, int index) {
            index = lua_absindex(L, index);
            if (lua_isstring(L, index)) {
               auto nk = lua_tostring(L, index);
               if (cobb::lua::is_metamethod_name(nk))
                  return true;
               else if (strcmp(nk, "__getters") == 0)
                  return true;
               else if (strcmp(nk, "__setters") == 0)
                  return true;
               else if (strcmp(nk, "__superclass") == 0)
                  return true;
               else if (strcmp(nk, "__name") == 0)
                  return true;
            }
            return false;
         }
      }
      static luastackchange_t __call(lua_State* L) {
         /*
         function(self, t, k)
            local k, v = next(self.names)
            while _should_skip_name(k) and _script_exists(t, k) do
               k, v = next(self.names)
            end
            if v ~= nil then
               return k, v
            end
            return
         end
         */
         // STACK: - [ self, t, k ] +
         constexpr auto index_self = 1;
         constexpr auto index_tbl  = 2;
         constexpr auto index_key  = 3;
         constexpr auto index_list = 4;
         constexpr auto index_nk   = 5;
         constexpr auto index_nv   = 6;
         //
         lua_getfield(L, index_self, "names");
         lua_pushvalue(L, index_key); // nk
         while (lua_next(L, index_self) != 0) {
            if (!_should_skip_name(L, index_nk)) {
               if (!false) { // if the papyrus root still has a script by this name
                  return 2;
               }
            }
            lua_settop(L, index_nk);
         }
         return 0;
      }
      //
      void _define_metatable(lua_State* L) {
         auto start   = lua_gettop(L);
         bool defined = luaL_getmetatable(L, metatable_key) == LUA_TTABLE;
         lua_settop(L, start);
         if (defined)
            return;
         luaL_newmetatable(L, metatable_key);
         auto index_mt = start + 1;
         //
         lua_pushcfunction(L, &__call);
         lua_setfield(L, index_mt, "__call");
         //
         lua_settop(L, start);
      }
   }
   #pragma endregion
}

namespace {
   using namespace editor_script;
   //
   namespace _methods {
   }
}

namespace editor_script::wrappers {
   /*static*/ luaL_Reg papyrus_root::metatable_methods[] = {
      { nullptr, nullptr },
   };
}