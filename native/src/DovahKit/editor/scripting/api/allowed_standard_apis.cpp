#include "allowed_standard_apis.h"
#include <array>
#include <cassert>
#include <vector>

namespace {
   using namespace editor_script;
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
   };
}

namespace editor_script {
   void prune_standard_library(lua_State* L, const char* libname) {
      for (auto& library : _libraries) {
         if (strcmp(library.name, libname) != 0)
            continue;
         auto base = lua_gettop(L);
         lua_pushnil(L); // STACK: [library, nil key]
         while (lua_next(L, base) != 0) {
            lua_pop(L, 1); // pop the value; keep the key for the next iteration
            if (lua_type(L, -2) != LUA_TSTRING) // skip the key if it isn't a string. apparently (lua_tostring) can modify the key, which confuses (lua_next)
               continue;
            auto* key = lua_tostring(L, -2);
            bool  any = false;
            for (auto* allowed_key : library.allowed_keys) {
               if (stricmp(key, allowed_key) == 0) {
                  any = true;
                  break;
               }
            }
            if (!any) { // this is not an allowed key
               lua_pushvalue(L, -2); // lua_settable pops its key, so we need to push a second copy
               lua_pushnil(L);
               lua_rawset(L, base);
            }
         }
      }
      lua_pop(L, 1); // pop the library table from the stack
   }
}