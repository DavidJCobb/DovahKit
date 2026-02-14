#pragma once
#include "dovahscript/api_helpers/fail_table_if_expandos.h"
#include "./member_spec.h"
#include "../alias.h"

namespace dovahscript::api_helpers::fill_params_helpers {
   template<const auto& MemberList, typename AliasType>
   int validate_table(AliasType& self_alias, lua_State* L, int stack_pos) {
      stack_pos = lua_absindex(L, stack_pos);

      constexpr const auto allowed_names = []() {
         std::array<std::string_view, MemberList.size() + 1> names = {};
         for (size_t i = 0; i < MemberList.size(); ++i)
            names[i] = MemberList[i].name;
         names[MemberList.size()] = "type";
         return names;
      }();
      api_helpers::fail_table_if_expandos(L, stack_pos, allowed_names);

      for (auto& m : MemberList) {
         lua_getfield(L, stack_pos, m.name.data());
         if (lua_isnoneornil(L, -1)) {
            lua_pop(L, 1);
            if (!m.optional_for_overwrite)
               return false;
            continue;
         }
         bool valid = m.validators.silently(self_alias, L, -1);
         lua_pop(L, 1);
         if (!valid)
            return false;
      }
      return true;
   }
}