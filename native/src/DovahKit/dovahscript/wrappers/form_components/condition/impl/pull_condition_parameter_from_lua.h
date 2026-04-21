#pragma once
#include <expected>
#include <string_view>
#include "dovah/data/conditions/event_function.h"
#include "dovah/forms/components/conditions/working_condition.h"
struct lua_State;
namespace dovahscript {
   class wrapper;
}

namespace dovahscript {
   extern std::expected<
      dovah::loaded_forms::components::conditions::working_parameter,
      std::string_view
   > pull_condition_parameter_from_lua(
      lua_State* L,
      int pos,
      wrapper&,
      const dovah::loaded_forms::components::conditions::working_condition&,
      int which_parameter
   );

   extern std::expected<dovah::conditions::event_function::type, std::string_view> pull_condition_event_function_from_lua(lua_State* L, int pos);
   extern std::expected<uint16_t, std::string_view> pull_condition_event_member_from_lua(lua_State* L, int pos);
   extern std::expected<dovah::form_stub*, std::string_view> pull_condition_event_form_from_lua(lua_State* L, int pos);
}