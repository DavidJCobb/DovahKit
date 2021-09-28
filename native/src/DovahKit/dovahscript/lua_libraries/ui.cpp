#include "dovah.h"
#include <array>
#include <cassert>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/setfuncs.h"
#include "../core/subsystems/coordinator.h"
#include "../send_script_task.h"

namespace {
   static constexpr const char* string_format_registry_key = "dovahscript.internal.dovah.string_format_copy";
}

namespace {
   using namespace dovahscript;
   
   namespace _definitions {
      int run_when_locked(lua_State* L) {
         cobb::lua::argcheck(L, lua_isfunction(L, 1), 1, "function expected");
         lua_settop(L, 1);
         core::subsystems::coordinator::get().queue_lua_function(1, true);
         return 0;
      }
      int run_when_unlocked(lua_State* L) {
         cobb::lua::argcheck(L, lua_isfunction(L, 1), 1, "function expected");
         lua_settop(L, 1);
         core::subsystems::coordinator::get().queue_lua_function(1, false);
         return 0;
      }
   }

   const std::initializer_list<luaL_Reg> _functions = {
      luaL_Reg{ "run_when_locked",   &_definitions::run_when_locked },
      luaL_Reg{ "run_when_unlocked", &_definitions::run_when_unlocked },
   };
}

namespace dovahscript::lua_libraries {
   namespace ui {
      extern void import(lua_State* L) {
         int t = lua_getglobal(L, "ui");
         if (t == LUA_TNONE || t == LUA_TNIL) {
            lua_createtable(L, 0, _functions.size());
            lua_pushvalue(L, -1);
            lua_setglobal(L, "ui");
         } else {
            assert(t == LUA_TTABLE && "How is the `ui` global not a table?!");
         }
         cobb::lua::setfuncs(L, _functions);
         lua_setglobal(L, "ui");
      }
   }
}