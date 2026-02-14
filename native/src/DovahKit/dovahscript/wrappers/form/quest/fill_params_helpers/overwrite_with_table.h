#pragma once
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"

namespace dovahscript::api_helpers::fill_params_helpers {
   template<const auto& MemberList, typename AliasType, typename FillParamsType>
   void overwrite_with_table(AliasType& self_alias, FillParamsType& fill, lua_State* L, int stack_pos) {
      core::subsystems::permissions::verify_form_write_permissions();

      stack_pos = lua_absindex(L, stack_pos);

      for (auto& m : MemberList) {
         lua_getfield(L, stack_pos, m.name.data());
         m.write(self_alias, fill, L, -1);
         lua_pop(L, 1);
      }
   }
}