#include "./push_pull_parameter_type_override.h"
#include "lua.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<parameter_type_override, std::string_view> pull_parameter_type_override(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      std::string_view v = lua_tostring(L, pos);
      if (v == "none") {
         return parameter_type_override::none;
      } else if (v == "alias") {
         return parameter_type_override::alias;
      } else if (v == "packdata") {
         return parameter_type_override::package_data;
      }
      return std::unexpected("unrecognized value");
   }

   extern void push_parameter_type_override(lua_State* L, parameter_type_override v) {
      switch (v) {
         case parameter_type_override::alias:
            lua_pushstring(L, "alias");
            return;
         case parameter_type_override::package_data:
            lua_pushstring(L, "packdata");
            return;
      }
      lua_pushstring(L, "none");
   }
   extern void push_parameter_type_override(lua_State* L, const condition_type& src) {
      if (src.test_flags(condition_type::flag::use_aliases)) {
         lua_pushstring(L, "alias");
      } else if (src.test_flags(condition_type::flag::use_package_data)) {
         lua_pushstring(L, "packdata");
      } else {
         lua_pushstring(L, "none");
      }
   }
}