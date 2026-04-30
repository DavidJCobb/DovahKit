#pragma once
#include <cstdint>
#include <expected>
#include <string_view>
#include "./_types.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   // Returns value or an error string.
   extern std::expected<uint16_t, std::string_view> pull_function_as_name(lua_State* L, int pos);
}