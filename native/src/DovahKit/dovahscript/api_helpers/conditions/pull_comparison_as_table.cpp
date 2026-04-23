#include "./pull_comparison_as_table.h"
#include <format>
#include "lua.h"
#include "./push_pull_comparison_operand.h"
#include "./push_pull_comparison_operator.h"
#include "../table_contains_expandos.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<working_comparison, std::string> pull_comparison_as_table(lua_State* L, int pos) {
      pos = lua_absindex(L, pos);
      switch (lua_type(L, pos)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            return std::unexpected("table or userdata expected");
      }
      {
         auto pair = table_contains_expandos(L, pos, std::array<std::string_view, 2>{ "operator", "operand" });
         if (pair.first) {
            if (!pair.second.empty()) {
               return std::unexpected(std::format("table contains one or more unexpected keys (first seen: `{}`)", pair.second));
            }
            return std::unexpected("table contains one or more unexpected keys");
         }
      }

      working_comparison cmp;
      {
         lua_getfield(L, pos, "operator");
         auto result = pull_comparison_operator(L, -1);
         lua_pop(L, 1);
         if (result.has_value())
            cmp.op = result.value();
         else
            return std::unexpected(std::string(result.error()));
      }
      {
         lua_getfield(L, pos, "operand");
         auto result = pull_comparison_operand(L, -1);
         lua_pop(L, 1);
         if (result.has_value())
            cmp.operand = result.value();
         else
            return std::unexpected(std::string(result.error()));
      }
      return cmp;
   }
}