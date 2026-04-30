#pragma once
#include <array>
#include <expected>
#include <optional>
#include <string>
#include "dovah/data/conditions/parameter_underlying_type.h"
#include "./_types.h"
namespace dovah::conditions {
   struct function_info;
   struct parameter_typeinfo;
}
struct lua_State;

namespace dovahscript::api_helpers::conditions {
   struct working_parameter_set {
      std::optional<working_type::event_data> event_data;
      std::array<working_parameter, 2> parameters;
   };

   // Returns value or an error string.
   extern std::expected<working_parameter_set, std::string> pull_parameters_as_table(
      lua_State* L,
      int pos,
      //
      const dovah::conditions::function_info&,
      const context_type&,
      parameter_type_override
   );


   // Treats `nil` keys in the table as "leave unchanged."
   // Returns an empty string or an error string.
   extern std::string pull_and_assign_parameters_as_table(
      lua_State* L,
      int pos,
      //
      const dovah::conditions::function_info&,
      const context_type&,
      parameter_type_override,
      //
      std::array<working_parameter, 2>&        dst_indexed,
      std::optional<working_type::event_data>& dst_event
   );
}