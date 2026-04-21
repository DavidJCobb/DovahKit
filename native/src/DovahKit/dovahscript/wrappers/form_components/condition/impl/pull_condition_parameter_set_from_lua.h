#pragma once
#include <array>
#include <expected>
#include <optional>
#include <string>
#include "dovah/forms/components/conditions/working_condition.h"
struct lua_State;
namespace dovahscript {
   class wrapper;
}

namespace dovahscript {
   namespace api_helpers {
      struct working_condition_parameter_set {
         using working_condition = dovah::loaded_forms::components::conditions::working_condition;
         using working_parameter = dovah::loaded_forms::components::conditions::working_parameter;

         std::optional<working_condition::event_data> event_data;
         std::array<working_parameter, 2> parameters;
      };
   }

   extern std::expected<api_helpers::working_condition_parameter_set, std::string> pull_condition_parameter_set_from_lua(
      lua_State* L,
      int pos,
      wrapper& self,
      const dovah::loaded_forms::components::conditions::working_condition&
   );
}