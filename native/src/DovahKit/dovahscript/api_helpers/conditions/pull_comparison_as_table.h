#pragma once
#include <expected>
#include <string_view>
#include "./_types.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   // Returns value or an error string.
   extern std::expected<working_comparison, std::string> pull_comparison_as_table(lua_State* L, int pos);

   // Treats `nil` keys in the table as "leave unchanged."
   // Returns an empty string or an error string.
   extern std::string pull_and_assign_comparison_as_table(lua_State* L, int pos, working_comparison& modify);
}