#pragma once
#include <expected>
#include <string_view>
#include "dovah/forms/components/conditions/working_condition.h"
#include "./_types.h"
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   // Returns value or an error string.
   extern std::expected<parameter_type_override, std::string_view> pull_parameter_type_override(lua_State* L, int pos);

   // Always pushes one value onto the Lua stack.
   extern void push_parameter_type_override(lua_State*, parameter_type_override);
   extern void push_parameter_type_override(lua_State*, const condition_type&);
}