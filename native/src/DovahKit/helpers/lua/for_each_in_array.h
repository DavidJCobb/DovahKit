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
#include <functional>
#include "../../lua.h"

namespace cobb::lua {
   // Given a Lua table, retrieves its length via the # operator; then, for each entry, pushes 
   // the entry onto the stack, calls the functor, and pops the entry. Functor takes the Lua 
   // state and the table entry index as arguments.
   //
   // Return codes:
   //  1 = No error, but length is zero
   //  0 = No error
   // -1 = Not a table
   // -2 = Non-integer length
   // -3 = Negative length
   [[nodiscard]] extern int for_each_in_array(lua_State* L, int stack_pos, std::function<void(lua_State*, int)> functor);
}
