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
   // Identical to luaL_error except that it's flagged as [[noreturn]], which may potentially 
   // allow for some compiler optimizations.
   [[noreturn]] inline void error(lua_State* L, const char* fmt, ...) noexcept(false) {
      va_list argp;
      va_start(argp, fmt);
      luaL_where(L, 1);
      lua_pushvfstring(L, fmt, argp);
      va_end(argp);
      lua_concat(L, 2);
      lua_error(L);
   }

   [[noreturn]] inline void argerror(lua_State* L, int arg, const char* message) noexcept(false) {
      luaL_argerror(L, arg, message);
   }

   // Identical to luaL_argcheck except that it's not a macro, and it relies on our [[noreturn]] 
   // argerror. This means that if you e.g. use this to error on a null pointer, IntelliSense 
   // should then know not to warn you about subsequent pointer access.
   inline void argcheck(lua_State* L, bool cond, int arg, const char* message) noexcept(false) {
      if (!cond)
         argerror(L, arg, message);
   }
}
