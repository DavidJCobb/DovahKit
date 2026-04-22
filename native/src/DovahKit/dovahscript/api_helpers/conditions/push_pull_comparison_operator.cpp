#include "./push_pull_comparison_operator.h"
#include "lua.h"

namespace dovahscript::api_helpers::conditions {
   // Returns run-on params or an error string.
   extern std::expected<dovah::conditions::comparison_operator, std::string_view> pull_comparison_operator(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected for operator");
      const auto v = std::string_view(lua_tostring(L, pos));
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

   // Always pushes one value onto the Lua stack.
   extern void push_comparison_operator(lua_State* L, dovah::conditions::comparison_operator v) {
      switch (v) {
         using enum dovah::conditions::comparison_operator;
         case equal:
            lua_pushstring(L, "==");
            return;
         case not_equal:
            lua_pushstring(L, "!=");
            return;
         case greater:
            lua_pushstring(L, ">");
            return;
         case greater_or_equal:
            lua_pushstring(L, ">=");
            return;
         case less:
            lua_pushstring(L, "<");
            return;
         case less_or_equal:
            lua_pushstring(L, "<=");
            return;
         default:
            lua_pushnil(L);
      }
   }
}