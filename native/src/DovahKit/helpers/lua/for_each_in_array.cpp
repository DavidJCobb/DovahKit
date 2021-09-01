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
#include "for_each_in_array.h"
#include "istablelike.h"

namespace cobb::lua {
   [[nodiscard]] extern int for_each_in_array(lua_State* L, int stack_pos, std::function<void(lua_State*, int)> functor) {
      lua_checkstack(L, 2);
      stack_pos = lua_absindex(L, stack_pos);
      if (!cobb::lua::istablelike(L, stack_pos))
         return -1;
      lua_len(L, stack_pos);
      int  isnum;
      auto count = lua_tointegerx(L, -1, &isnum);
      if (!isnum)
         return -2;
      if (count < 0)
         return -3;
      if (count == 0)
         return 1;
      lua_pop(L, 1);
      //
      for (lua_Integer i = 1; i <= count; ++i) {
         lua_geti(L, stack_pos, i);
         (functor)(L, i);
         lua_pop(L, 1);
      }
      return 0;
   }
}