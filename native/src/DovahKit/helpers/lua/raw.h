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
#pragma once
#include "../../lua.h"

namespace cobb::lua {
   inline int rawgetfield(lua_State* L, int table_pos, const char* field) noexcept {
      table_pos = lua_absindex(L, table_pos);
      lua_pushstring(L, field);
      return lua_rawget(L, table_pos);
   }

   // Given a value at the top of the stack, write it to table_pos[field].
   inline void rawsetfield(lua_State* L, int table_pos, const char* field) noexcept {
      table_pos = lua_absindex(L, table_pos);
      lua_pushstring(L, field);
      lua_rotate(L, -2, 1);
      lua_rawset(L, table_pos);
   }

   inline int rawgetvalue(lua_State* L, int table_pos, int key_pos) noexcept {
      table_pos = lua_absindex(L, table_pos);
      lua_pushvalue(L, key_pos);
      return lua_rawget(L, table_pos);
   }
}