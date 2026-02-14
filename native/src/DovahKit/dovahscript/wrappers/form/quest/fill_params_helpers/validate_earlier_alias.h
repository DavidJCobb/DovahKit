#pragma once
#include <type_traits>
#include "helpers/lua/error.h"
#include "dovah/forms/Quest.h"
#include "lua.h"
#include "../alias.h"

namespace dovahscript::api_helpers::fill_params_helpers {
   using alias_type = dovah::loaded_forms::Alias::alias_type;

   template<bool Complain, bool Optional, alias_type Type>
   extern std::conditional_t<Complain, void, bool> validate_earlier_alias(
      const dovah::loaded_forms::Alias& alias,
      lua_State* L,
      int stack_pos
   ) {
      constexpr const auto expectation = (Type == alias_type::reference) ? "ref alias expected" : "loc alias expected";

      stack_pos = lua_absindex(L, stack_pos);

      if (lua_isnoneornil(L, stack_pos)) {
         if constexpr (Complain)
            cobb::lua::argerror(L, stack_pos, expectation);
         else
            return Optional;
      }

      auto* value_wrap = (wrapper*) dovahscript::classes::cast_to_class(L, stack_pos, wrappers::quest_alias::metatable_key);
      if (!value_wrap) {
         if constexpr (Complain)
            cobb::lua::argerror(L, stack_pos, expectation);
         else
            return false;
      }
      auto* value_alias = wrappers::quest_alias::unwrap(*value_wrap);
      if (!value_alias || value_alias->type != Type) {
         if constexpr (Complain)
            cobb::lua::argerror(L, stack_pos, expectation);
         else
            return false;
      }
      if (&value_alias->owner != &alias.owner) {
         if constexpr (Complain)
            cobb::lua::argerror(L, stack_pos, "you must specify an alias from the same quest");
         else
            return false;
      }
      if (value_alias->id >= alias.id) {
         if constexpr (Complain)
            cobb::lua::argerror(L, stack_pos, "you must specify an alias with a lower ID than this alias's ID");
         else
            return false;
      }

      if constexpr (!Complain) {
         return true;
      }
   }
}