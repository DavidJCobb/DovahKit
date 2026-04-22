#include "./pull_parameters_as_table.h"
#include <cassert>
#include "lua.h"
#include "dovah/data/conditions/all_parameter_types.h"
#include "dovah/data/conditions/function_info.h"
#include "dovah/data/conditions/parameter_typeinfo.h"
#include "./push_pull_event_parameter.h"
#include "./push_pull_indexed_parameter.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<working_parameter_set, std::string> pull_parameters_as_table(
      lua_State* L,
      int table_pos,
      //
      const dovah::conditions::function_info& function_info,
      const context_type& context,
      parameter_type_override pto
   ) {
      switch (lua_type(L, table_pos)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            return std::unexpected("value is not a table or userdata");
      }

      working_parameter_set dst;
      
      if (function_info.uses_event_data) {
         auto& ep = dst.event_data.emplace();

         #define CASE(name) \
            {                                          \
               lua_getfield(L, -1, #name);             \
               auto result = pull_event_##name(L, -1); \
               lua_pop(L, 1);                          \
               if (result.has_value())                 \
                  ep.name = result.value();            \
               else {                                  \
                  auto error = std::format("problem with `" #name "`: {}", result.error()); \
                  return std::unexpected(error);       \
               }                                       \
            }
         CASE(function)
         CASE(member)
         CASE(form)
         #undef CASE
      } else {
         for (size_t i = 0; i < 2; ++i) {
            auto* typeinfo = [pto, i, &dst, &function_info]() -> const dovah::conditions::parameter_typeinfo* {
               auto* typeinfo = function_info.argument_types[i];
               if (!typeinfo)
                  return nullptr;
               if (i == 1 && typeinfo->is_union()) {
                  auto& union_info = typeinfo->union_decider.value();
                  assert(function_info.argument_types[0] == union_info.decide_by);
                  auto& prev = dst.parameters[0];
                  if (!std::holds_alternative<int32_t>(prev))
                     return nullptr;
                  typeinfo = (union_info.decider)(std::get<int32_t>(prev));
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
            }();
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

      return dst;
   }
}