#pragma once
#include <expected>
#include <string_view>
#include "dovah/data/conditions/parameter_underlying_type.h"
#include "dovah/forms/components/conditions/working_parameter.h"
#include "./_types.h"
namespace dovah::conditions {
   struct parameter_typeinfo;
}
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   extern std::expected<
      working_parameter,
      std::string_view
   > pull_indexed_parameter(
      lua_State* L,
      int pos,
      //
      const context_type&,
      dovah::conditions::parameter_underlying_type,
      const dovah::conditions::parameter_typeinfo*
   );

   // Always pushes one value onto the Lua stack.
   extern void push_indexed_parameter(
      lua_State*,
      const context_type&,
      const dovah::conditions::parameter_typeinfo*,
      const dovah::conditions::parameter_underlying_type,
      const working_parameter&
   );
}