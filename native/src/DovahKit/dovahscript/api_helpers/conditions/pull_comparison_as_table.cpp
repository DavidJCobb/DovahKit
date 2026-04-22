#include "./pull_comparison_as_table.h"
#include "lua.h"
#include "./push_pull_comparison_operand.h"
#include "./push_pull_comparison_operator.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<working_comparison, std::string_view> pull_comparison_as_table(lua_State* L, int pos) {
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
         auto result = pull_comparison_operator(L, -1);
         lua_pop(L, 1);
         if (result.has_value())
            cmp.op = result.value();
         else
            return std::unexpected(std::move(result.error()));
      }
      {
         lua_getfield(L, pos, "operand");
         auto result = pull_comparison_operand(L, -1);
         lua_pop(L, 1);
         if (result.has_value())
            cmp.operand = result.value();
         else
            return std::unexpected(std::move(result.error()));
      }
      return cmp;
   }
}