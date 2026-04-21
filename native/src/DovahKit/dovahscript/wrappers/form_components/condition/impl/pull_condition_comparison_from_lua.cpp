#include "./pull_condition_comparison_from_lua.h"
#include "lua.h"
#include "dovahscript/wrappers/form/form.h"
#include "dovahscript/wrapper.h"

namespace {
   using working_comparison = dovah::loaded_forms::components::conditions::working_comparison;
   using working_operand    = decltype(working_comparison::operand);
}

namespace dovahscript {
   extern std::expected<dovah::conditions::comparison_operator, std::string_view> pull_condition_comparison_operator_from_lua(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected for operator");
      const auto v      = std::string_view(lua_tostring(L, pos));
      using enumeration = dovah::conditions::comparison_operator;
      if (v == "==")
         return enumeration::equal;
      else if (v == "!=")
         return enumeration::not_equal;
      else if (v == ">")
         return enumeration::greater;
      else if (v == ">=")
         return enumeration::greater_or_equal;
      else if (v == "<")
         return enumeration::less;
      else if (v == "<=")
         return enumeration::less_or_equal;
      return std::unexpected("unrecognized operator");
   }

   extern std::expected<working_operand, std::string_view> pull_condition_comparison_operand_from_lua(lua_State* L, int pos) {
      if (lua_isnumber(L, pos)) {
         return (float)lua_tonumber(L, pos);
      }
      auto* other = wrapper_from_stack<wrappers::form>(L, pos);
      if (!other || !other->stub || other->stub->form_type != dovah::form_type::global)
         return std::unexpected("operand must be a number or a form of type `global`");
      return other->stub;
   }

   extern std::expected<working_comparison, std::string_view> pull_condition_comparison_from_lua(lua_State* L, int pos) {
      switch (lua_type(L, pos)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            return std::unexpected("table or userdata expected");
      }

      working_comparison cmp;
      {
         lua_getfield(L, pos, "operator");
         auto result = pull_condition_comparison_operator_from_lua(L, -1);
         lua_pop(L, 1);
         if (result.has_value())
            cmp.op = result.value();
         else
            return std::unexpected(std::move(result.error()));
      }
      {
         lua_getfield(L, pos, "operand");
         auto result = pull_condition_comparison_operand_from_lua(L, -1);
         lua_pop(L, 1);
         if (result.has_value())
            cmp.operand = result.value();
         else
            return std::unexpected(std::move(result.error()));
      }
      return cmp;
   }
}