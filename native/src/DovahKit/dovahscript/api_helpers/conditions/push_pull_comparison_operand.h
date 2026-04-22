#pragma once
#include <expected>
#include <string_view>
#include "./_types.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   // Returns value or an error string.
   extern std::expected<decltype(working_comparison::operand), std::string_view> pull_comparison_operand(lua_State* L, int pos);

   // Always pushes one value onto the Lua stack.
   extern void push_comparison_operand(lua_State*, const decltype(condition_type::comparison_data::operand)&);
}