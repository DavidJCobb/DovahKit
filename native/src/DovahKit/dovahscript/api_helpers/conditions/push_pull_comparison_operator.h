#pragma once
#include <expected>
#include <string_view>
#include "dovah/data/conditions/comparison_operator.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   // Returns value or an error string.
   extern std::expected<dovah::conditions::comparison_operator, std::string_view> pull_comparison_operator(lua_State* L, int pos);

   // Always pushes one value onto the Lua stack.
   extern void push_comparison_operator(lua_State*, dovah::conditions::comparison_operator);
}