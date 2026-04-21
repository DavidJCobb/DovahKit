#pragma once
#include <expected>
#include <string_view>
#include "dovah/forms/components/conditions/working_comparison.h"
struct lua_State;

namespace dovahscript {
   extern std::expected<dovah::conditions::comparison_operator, std::string_view> pull_condition_comparison_operator_from_lua(lua_State* L, int pos);

   extern std::expected<
      decltype(dovah::loaded_forms::components::conditions::working_comparison::operand),
      std::string_view
   > pull_condition_comparison_operand_from_lua(lua_State* L, int pos);

   extern std::expected<dovah::loaded_forms::components::conditions::working_comparison, std::string_view> pull_condition_comparison_from_lua(lua_State* L, int pos);
}