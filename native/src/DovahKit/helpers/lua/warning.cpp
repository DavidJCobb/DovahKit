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
#include "warning.h"

namespace cobb::lua {
   void warning(lua_State* L, const char* fmt, ...) {
      va_list argp;
      va_start(argp, fmt);
      lua_checkstack(L, 2);
      luaL_where(L, 1);
      lua_pushvfstring(L, fmt, argp);
      va_end(argp);
      lua_concat(L, 2);
      auto* str = lua_tostring(L, -1);
      lua_warning(L, str, 0);
      lua_pop(L, 1);
   }
}
