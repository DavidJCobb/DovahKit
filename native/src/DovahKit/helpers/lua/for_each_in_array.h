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
