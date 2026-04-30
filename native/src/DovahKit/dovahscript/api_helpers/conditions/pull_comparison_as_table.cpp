#include "./pull_comparison_as_table.h"
#include <format>
#include "lua.h"
#include "./push_pull_comparison_operand.h"
#include "./push_pull_comparison_operator.h"
#include "../table_contains_expandos.h"

#include "./common/FAIL_UNLESS_TABLE_LIKE.h"
#include "./common/FAIL_ON_EXPANDOS.h"
#include "./common/TRY_ASSIGN_FIELD.h"
#include "./common/TRY_OVERWRITE_FIELD.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<working_comparison, std::string> pull_comparison_as_table(lua_State* L, int pos) {
      pos = lua_absindex(L, pos);
      #define FAIL(v) return std::unexpected(v);
      FAIL_UNLESS_TABLE_LIKE(L, pos);
      FAIL_ON_EXPANDOS(L, pos, "operator", "operand");

      #undef FAIL
      #define FAIL(v) return std::unexpected(std::string(v));

      working_comparison cmp;
      TRY_OVERWRITE_FIELD(cmp, op,      "operator", L, pos, pull_comparison_operator);
      TRY_OVERWRITE_FIELD(cmp, operand, "operand",  L, pos, pull_comparison_operand);

      #undef FAIL
      return cmp;
   }

   extern std::string pull_and_assign_comparison_as_table(lua_State* L, int pos, working_comparison& modify) {
      pos = lua_absindex(L, pos);
      #define FAIL(v) return v;
      FAIL_UNLESS_TABLE_LIKE(L, pos);
      FAIL_ON_EXPANDOS(L, pos, "operator", "operand");

      #undef FAIL
      #define FAIL(v) return std::string(v);

      TRY_ASSIGN_FIELD(modify, op,      "operator", L, pos, pull_comparison_operator);
      TRY_ASSIGN_FIELD(modify, operand, "operand",  L, pos, pull_comparison_operand);

      #undef FAIL
      return {};
   }
}