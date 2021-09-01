/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "setfuncs.h"

namespace cobb::lua {
   void setfuncs(lua_State* L, const std::initializer_list<luaL_Reg>& list) {
      auto top = lua_gettop(L);
      for (const auto& entry : list) {
         if (!entry.name)
            continue;
         if (!entry.func) { // placeholder
            lua_pushboolean(L, 0);
            lua_setfield(L, top, entry.name);
            continue;
         }
         lua_pushcfunction(L, entry.func);
         lua_setfield(L, top, entry.name);
      }
   }
   void setfuncs(lua_State* L, const std::initializer_list<luaL_Reg>& list, int upvalues) {
      auto table = lua_gettop(L) - upvalues;
      for (const auto& entry : list) {
         if (!entry.name)
            continue;
         if (!entry.func) { // placeholder
            lua_pushboolean(L, 0);
            lua_setfield(L, table, entry.name);
            continue;
         }
         for (int i = 0; i < upvalues; ++i)
            lua_pushvalue(L, -upvalues);
         lua_pushcfunction(L, entry.func);
         lua_setfield(L, table, entry.name);
      }
      lua_pop(L, upvalues);
   }
   void setfuncs(lua_State* L, const std::vector<luaL_Reg>& list) {
      auto top = lua_gettop(L);
      for (const auto& entry : list) {
         if (!entry.name)
            continue;
         if (!entry.func) { // placeholder
            lua_pushboolean(L, 0);
            lua_setfield(L, top, entry.name);
            continue;
         }
         lua_pushcfunction(L, entry.func);
         lua_setfield(L, top, entry.name);
      }
   }
   void setfuncs(lua_State* L, const std::vector<luaL_Reg>& list, int upvalues) {
      auto table = lua_gettop(L) - upvalues;
      for (const auto& entry : list) {
         if (!entry.name)
            continue;
         if (!entry.func) { // placeholder
            lua_pushboolean(L, 0);
            lua_setfield(L, table, entry.name);
            continue;
         }
         for (int i = 0; i < upvalues; ++i)
            lua_pushvalue(L, -upvalues);
         lua_pushcfunction(L, entry.func);
         lua_setfield(L, table, entry.name);
      }
      lua_pop(L, upvalues);
   }
}