#include "all_standard.h"
#include <array>
#include <initializer_list>
#include <vector>
#include "../../helpers/lua/error.h"
#include "../core/subsystems/coordinator.h"
#include "../tasks/s2m/log_message.h"

namespace {
   #pragma region Pruning the standard libraries
      struct _lib {
         const char* name;
         std::vector<const char*> allowed_keys;
         //
         _lib(const char* n, std::initializer_list<const char*> l) : name(n), allowed_keys(l) {}
      };

      std::array _libraries = {
         //
         // This array should contain only libraries that we want to whitelist functions for. If we 
         // want to include a library and not bother pruning its functions at all, then we shouldn't 
         // list that library here.
         //
         _lib( "basic", {
            "assert",
            "collectgarbage", // but we're gonna override it
            "error",
            "_G",
            "getmetatable",
            "ipairs",
            "next",
            "pairs",
            "pcall", // but we're gonna override it
            "print",
            "rawequal",
            "rawget",
            "rawlen",
            "rawset",
            "select",
            "setmetatable",
            "tonumber",
            "tostring",
            "type",
            "_VERSION",
            "warn",
         }),
         _lib( "debug", {
            // the debug metatable APIs don't protect userdata, so don't whitelist them
            "traceback",
         }),
         _lib("os", {
            "clock",
         }),
      };

      
      void _prune_standard_library(lua_State* L, const char* libname) {
         for (auto& library : _libraries) {
            if (strcmp(library.name, libname) != 0)
               continue;
            auto base = lua_gettop(L);
            lua_pushnil(L); // STACK: [library, nil key]
            while (lua_next(L, base) != 0) {
               lua_pop(L, 1); // pop the value; keep the key for the next iteration
               if (lua_type(L, -1) != LUA_TSTRING) // skip the key if it isn't a string. apparently (lua_tostring) can modify the key, which confuses (lua_next)
                  continue;
               auto* key = lua_tostring(L, -1);
               bool  any = false;
               for (auto* allowed_key : library.allowed_keys) {
                  if (stricmp(key, allowed_key) == 0) {
                     any = true;
                     break;
                  }
               }
               if (!any) { // this is not an allowed key
                  lua_pushvalue(L, -1); // lua_settable pops its key, so we need to push a second copy
                  lua_pushnil(L);
                  lua_rawset(L, base);
               }
            }
         }
         lua_pop(L, 1); // pop the library table from the stack
      }
   #pragma endregion

   namespace _shims {
      int collectgarbage(lua_State* L) {
         luaL_argcheck(L, lua_isstring(L, 1), 1, "The argument must be a string.");
         if (strcmp(lua_tostring(L, 1), "collect") != 0)
            cobb::lua::error(L, "The only garbage-collection feature that this script environment allows access to is \"collect\".");
         lua_gc(L, LUA_GCCOLLECT);
         return 0;
      }
      int pcall(lua_State* L) { // (pcall) shim to prevent userscripts from catching the error that (_lua_debug_hook) uses to force-kill a script
         int arg_count = lua_gettop(L) - 1;
         lua_pushboolean(L, true); // pcall result code if we don't hit an error
         lua_insert(L, 1);         // move the result code before the function and args
         int status = lua_pcall(L, arg_count, LUA_MULTRET, 0);
         if (status != LUA_OK) {
            //
            // Stack now contains only an error object.
            //
            if (lua_type(L, -1) == LUA_TUSERDATA) {
               lua_getfield(L, LUA_REGISTRYINDEX, dovahscript::core::subsystems::coordinator::abort_sentinel_userdata);
               bool eq = lua_rawequal(L, -1, -2);
               lua_pop(L, 1);
               if (eq)
                  //
                  // This is the userdata sentinel object we use when the user calls an abort. We should not 
                  // allow the script to catch this error, as it is the means through which we forcibly halt 
                  // script execution.
                  //
                  lua_error(L); // re-throw
            }
            //
            // The error is already on the stack, so let's just push the success bool 
            // and error text, and then we oughta be good.
            //
            lua_pushboolean(L, false); // stack after this: [true, error, false]
            lua_pushstring(L, lua_tostring(L, -2)); // stack after this: [true, error, false, "error"]
            return 2;
         }
         return lua_gettop(L); // return value count + the boolean we pushed
      }
      int print(lua_State* L) {
         auto  m    = new dovahscript::tasks::s2m::log_message();
         auto& text = m->text;
         //
         auto argcount = lua_gettop(L);
         for (int i = 1; i <= argcount; ++i) {
            size_t length;
            auto*  content = luaL_tolstring(L, i, &length);
            if (i > 1)
               text += '\t';
            text += QString::fromUtf8(content, length);
            lua_pop(L, 1);
         }
         //
         dovahscript::core::subsystems::coordinator::get().send_script_task(*m);
         return 0;
      }
   }
}

namespace dovahscript::lua_libraries {
   namespace all_standard {
      extern void import(lua_State* L) {
         luaL_requiref(L, "_G", luaopen_base, 1); // loads the library to the top of the Lua stack
         {  // shim collectgarbage
            auto ti = lua_gettop(L);
            lua_pushstring(L, "collectgarbage");
            lua_pushcfunction(L, &_shims::collectgarbage);
            lua_rawset(L, ti);
         }
         {  // shim pcall
            auto ti = lua_gettop(L);
            lua_pushstring(L, "pcall");
            lua_pushcfunction(L, &_shims::pcall);
            lua_rawset(L, ti);
         }
         {  // shim print
            auto ti = lua_gettop(L);
            lua_pushstring(L, "print");
            lua_pushcfunction(L, &_shims::print);
            lua_rawset(L, ti);
         }
         _prune_standard_library(L, "basic"); // also pops the library from the Lua stack
         luaL_requiref(L, "debug", luaopen_debug, 1);
         _prune_standard_library(L, "debug");
         luaL_requiref(L, "math", luaopen_math, 1);
         _prune_standard_library(L, "math");
         luaL_requiref(L, "string", luaopen_string, 1);
         _prune_standard_library(L, "string");
         luaL_requiref(L, "table", luaopen_table, 1);
         _prune_standard_library(L, "table");
         luaL_requiref(L, "utf8", luaopen_utf8, 1);
         _prune_standard_library(L, "utf8");
      }
   }
}