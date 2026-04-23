#pragma once
#include <array>
#include <format>
#include <string_view>
#include "helpers/lua/error.h"
#include "./table_contains_expandos.h"

namespace dovahscript::api_helpers {
   template<size_t Size>
   void fail_table_if_expandos(lua_State* L, int arg, const std::array<std::string_view, Size>& allowed_keys) {
      auto pair = table_contains_expandos(L, arg, allowed_keys);
      if (pair.first) {
         if (pair.second.empty())
            cobb::lua::argerror(L, arg, "table contains one or more unexpected keys");
         auto error = std::format("table contains one or more unexpected keys (first seen: `{}`)", pair.second);
         cobb::lua::argerror(L, arg, error.c_str());
      }
   }
}