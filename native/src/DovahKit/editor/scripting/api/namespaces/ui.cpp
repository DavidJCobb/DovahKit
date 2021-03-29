#include "ui.h"
#include "../../editor_script_core.h"

namespace {
   using namespace editor_script;

   namespace _definitions {
      luastackchange_t lock(lua_State* L) {
         DovahKitScriptVMUITaskConduit::get().set_ui_lock_state_override(true);
         return 0;
      }
      luastackchange_t unlock(lua_State* L) {
         DovahKitScriptVMUITaskConduit::get().set_ui_lock_state_override(false);
         return 0;
      }
      luastackchange_t set_lock_state(lua_State* L) {
         luaL_argcheck(L, lua_isboolean(L, 1), 1, "boolean expected");
         DovahKitScriptVMUITaskConduit::get().set_ui_lock_state_override(lua_toboolean(L, 1));
         return 0;
      }
   }

   std::array _functions = {
      luaL_Reg{ "lock",           &_definitions::lock },
      luaL_Reg{ "unlock",         &_definitions::unlock },
      luaL_Reg{ "set_lock_state", &_definitions::set_lock_state },
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
