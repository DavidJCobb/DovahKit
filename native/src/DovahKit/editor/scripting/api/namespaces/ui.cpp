#include "ui.h"
#include "../../systems/editor_script_inner_core.h"

namespace {
   using namespace editor_script;

   namespace _definitions {
      luastackchange_t run_when_locked(lua_State* L) {
         luaL_argcheck(L, lua_isfunction(L, 1), 1, "function expected");
         lua_settop(L, 1);
         DovahKitScriptVMCore::get().queue_lua_function(1, true);
         return 0;
      }
      luastackchange_t run_when_unlocked(lua_State* L) {
         luaL_argcheck(L, lua_isfunction(L, 1), 1, "function expected");
         lua_settop(L, 1);
         DovahKitScriptVMCore::get().queue_lua_function(1, false);
         return 0;
      }
   }

   std::array _functions = {
      luaL_Reg{ "run_when_locked",   &_definitions::run_when_locked },
      luaL_Reg{ "run_when_unlocked", &_definitions::run_when_unlocked },
   };
}
namespace editor_script::namespace_setup {
   extern void ui(lua_State* L) {
      int pos = lua_gettop(L);
      for (auto& entry : _functions) {
         lua_pushstring(L, entry.name);    // key
         lua_pushcfunction(L, entry.func); // value
         lua_rawset(L, pos);
      }
   }
}
