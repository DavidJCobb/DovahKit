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
#include "find_key_of.h"

namespace cobb::lua {
   [[nodiscard]] extern std::string find_key_of(lua_State* L, int table_index, int value_index) {
      /*
      function _table_has_value(t, value)
         local k, v = next(t)
         while k do
            if v == value then
               return k
            end
            k, v = next(t, k)
         end
         return ""
      end
      */
      table_index = lua_absindex(L, table_index);
      value_index = lua_absindex(L, value_index);
      if (!lua_istable(L, table_index))
         return std::string();
      lua_pushnil(L);
      while (lua_next(L, table_index) != 0) {
         if (lua_rawequal(L, -1, value_index)) {
            lua_pop(L, 1); // pop value
            std::string name = lua_tostring(L, -1);
            lua_pop(L, 1); // pop key
            return name;
         }
         lua_pop(L, 1); // pop value for lua_next
      }
      return std::string();
   }
}