#include "./pull_parameters_as_table.h"
#include <cassert>
#include "lua.h"
#include "dovah/data/conditions/all_parameter_types.h"
#include "dovah/data/conditions/function_info.h"
#include "dovah/data/conditions/parameter_typeinfo.h"
#include "./push_pull_event_parameter.h"
#include "./push_pull_indexed_parameter.h"

#include "./common/FAIL_UNLESS_TABLE_LIKE.h"
#include "./common/TRY_ASSIGN_FIELD.h"
#include "./common/TRY_OVERWRITE_FIELD.h"

namespace {
   using function_info_type      = dovah::conditions::function_info;
   using parameter_typeinfo      = dovah::conditions::parameter_typeinfo;
   using parameter_type_override = dovah::loaded_forms::components::conditions::parameter_type_override;
   
   static const parameter_typeinfo* typeinfo_of_parameter_a(const function_info_type& function_info, parameter_type_override pto) {
      const auto* typeinfo = function_info.argument_types[0];
      if (!typeinfo)
         return nullptr;
      assert(!typeinfo->is_union());
      if (typeinfo->allow_type_overrides) {
         switch (pto) {
            case parameter_type_override::alias:
               return &dovah::conditions::parameter_types::Alias;
            case parameter_type_override::package_data:
               return &dovah::conditions::parameter_types::PackageData;
         }
      }
      return typeinfo;
   }
   static const parameter_typeinfo* typeinfo_of_parameter_b(
      const function_info_type& function_info,
      parameter_type_override   pto,
      const dovah::loaded_forms::components::conditions::working_parameter&  param_a_value
   ) {
      const auto* typeinfo = function_info.argument_types[1];
      if (!typeinfo)
         return nullptr;
      if (typeinfo->is_union()) {
         auto& union_info = typeinfo->union_decider.value();
         assert(function_info.argument_types[0] == union_info.decide_by);
         if (!std::holds_alternative<int32_t>(param_a_value))
            return nullptr;
         typeinfo = (union_info.decider)(std::get<int32_t>(param_a_value));
         if (!typeinfo)
            return nullptr;
      }
      if (typeinfo->allow_type_overrides) {
         switch (pto) {
            case parameter_type_override::alias:
               return &dovah::conditions::parameter_types::Alias;
            case parameter_type_override::package_data:
               return &dovah::conditions::parameter_types::PackageData;
         }
      }
      return typeinfo;
   }
}

namespace dovahscript::api_helpers::conditions {
   extern std::expected<working_parameter_set, std::string> pull_parameters_as_table(
      lua_State* L,
      int table_pos,
      //
      const dovah::conditions::function_info& function_info,
      const context_type& context,
      parameter_type_override pto
   ) {
      table_pos = lua_absindex(L, table_pos);
      #define FAIL(v) return std::unexpected(v);
      FAIL_UNLESS_TABLE_LIKE(L, table_pos);

      working_parameter_set dst;
      
      if (function_info.uses_event_data) {
         auto& ep = dst.event_data.emplace();
         #define CASE(name) TRY_OVERWRITE_FIELD_WITH_DIAGNOSTIC(ep, name, #name, L, table_pos, pull_event_##name)
         CASE(function)
         CASE(member)
         CASE(form)
         #undef CASE
      } else {
         for (size_t i = 0; i < 2; ++i) {
            const auto* typeinfo = (i == 0) ?
               typeinfo_of_parameter_a(function_info, pto)
            :
               typeinfo_of_parameter_b(function_info, pto, dst.parameters[0]);
            if (!typeinfo)
               typeinfo = &dovah::conditions::parameter_types::None;

            lua_geti(L, -1, i + 1);
            auto result = pull_indexed_parameter(
               L,
               -1,
               context,
               typeinfo->underlying_type,
               typeinfo
            );
            lua_pop(L, 1);
            if (result.has_value()) {
               dst.parameters[i] = result.value();
            } else {
               auto error = std::format("problem with parameter #{}: {}", i + 1, result.error());
               return std::unexpected(error);
            }
         }
      }

      #undef FAIL
      return dst;
   }

   extern std::string pull_and_assign_parameters_as_table(
      lua_State* L,
      int table_pos,
      //
      const dovah::conditions::function_info& func_info,
      const context_type& context,
      parameter_type_override pto,
      //
      std::array<working_parameter, 2>& dst_indexed,
      std::optional<working_type::event_data>& dst_event
   ) {
      table_pos = lua_absindex(L, table_pos);
      #define FAIL(v) return v;
      FAIL_UNLESS_TABLE_LIKE(L, table_pos);
      
      if (func_info.uses_event_data) {
         auto& ep = dst_event.emplace();
         #define CASE(name) TRY_ASSIGN_FIELD_WITH_DIAGNOSTIC(ep, name, #name, L, table_pos, pull_event_##name)
         CASE(function)
         CASE(member)
         CASE(form)
         #undef CASE
      } else {
         for (size_t i = 0; i < 2; ++i) {
            lua_geti(L, -1, i + 1);
            if (lua_isnoneornil(L, -1)) {
               lua_pop(L, 1);
               continue;
            }

            const auto* typeinfo = (i == 0) ?
               typeinfo_of_parameter_a(func_info, pto)
            :
               typeinfo_of_parameter_b(func_info, pto, dst_indexed[0]);
            if (!typeinfo)
               typeinfo = &dovah::conditions::parameter_types::None;

            auto result = pull_indexed_parameter(
               L,
               -1,
               context,
               typeinfo->underlying_type,
               typeinfo
            );
            lua_pop(L, 1);
            if (result.has_value()) {
               dst_indexed[i] = result.value();
            } else {
               auto error = std::format("problem with parameter #{}: {}", i + 1, result.error());
               return error;
            }
         }
      }

      #undef FAIL
      return {};
   }
}