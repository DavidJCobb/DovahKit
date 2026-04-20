#pragma once
#include <array>
#include <string_view>
#include "helpers/lua/error.h"
#include "./table_contains_expandos.h"

namespace dovahscript::api_helpers {
   template<size_t Size>
   void fail_table_if_expandos(lua_State* L, int arg, const std::array<std::string_view, Size>& allowed_keys) {
      if (table_contains_expandos(L, arg, allowed_keys))
         cobb::lua::argerror(L, arg, "table contained an unexpected key");
   }
}