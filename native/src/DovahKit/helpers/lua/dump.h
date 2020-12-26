#pragma once
#include <string>

struct lua_State;

namespace cobb::lua {
   extern void print_stack(lua_State*, int stack_start = 1, int stack_end = -1);
   extern void print_stack_and_vars(lua_State*, int stack_start = 1, int stack_end = -1);
   extern std::string var_to_string(lua_State*, int pos);
}