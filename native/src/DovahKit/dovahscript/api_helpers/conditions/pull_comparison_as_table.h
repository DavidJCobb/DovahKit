#pragma once
#include <expected>
#include <string_view>
#include "./_types.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   // Returns value or an error string.
   extern std::expected<working_comparison, std::string> pull_comparison_as_table(lua_State* L, int pos);
}