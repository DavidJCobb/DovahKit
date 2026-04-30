#include "./pull_function_as_name.h"
#include "helpers/string/strieq_ascii.h"
#include "lua.h"
#include "dovah/data/conditions/all_function_info.h"

namespace dovahscript::api_helpers::conditions {
   std::expected<uint16_t, std::string_view> pull_function_as_name(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      const std::string_view arg = lua_tostring(L, pos);
      for (const auto& info : dovah::conditions::all_vanilla_function_info)
         if (cobb::strieq_ascii(info.name, arg))
            return info.id;
      for (const auto& info : dovah::conditions::all_extended_function_info)
         if (cobb::strieq_ascii(info.name, arg))
            return info.id;
      return std::unexpected("unrecognized function name");
   }
}