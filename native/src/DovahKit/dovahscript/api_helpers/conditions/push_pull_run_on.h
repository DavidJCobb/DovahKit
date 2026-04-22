#pragma once
#include <expected>
#include <string_view>
#include "./_types.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   using working_run_on_params = decltype(working_type::run_on);

   // Returns run-on params or an error string.
   extern std::expected<working_run_on_params, std::string_view> pull_run_on(lua_State* L, int pos, const context_type&);

   // Always pushes one value onto the Lua stack.
   extern void push_run_on(lua_State* L, const condition_type::run_on_data&, const context_type&);
}