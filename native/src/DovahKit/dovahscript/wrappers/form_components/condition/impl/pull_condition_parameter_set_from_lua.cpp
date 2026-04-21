#include "./pull_condition_parameter_set_from_lua.h"
#include <format>
#include "helpers/lua/error.h"
#include "lua.h"
#include "dovah/data/conditions/all_function_info.h"
#include "./pull_condition_parameter_from_lua.h"

namespace dovahscript {
   extern std::expected<api_helpers::working_condition_parameter_set, std::string> pull_condition_parameter_set_from_lua(
      lua_State* L,
      int table_pos,
      wrapper& self,
      const dovah::loaded_forms::components::conditions::working_condition& working
   ) {
      switch (lua_type(L, table_pos)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            return std::unexpected("value is not a table or userdata");
      }

      api_helpers::working_condition_parameter_set dst;
      
      const auto* info = dovah::conditions::function_info_by_id(working.function);
      if (info && info->uses_event_data) {
         dst.event_data = working.event_parameters;
         if (!dst.event_data.has_value())
            dst.event_data.emplace();
         auto& ep = dst.event_data.value();

         #define CASE(name) \
            {                                                               \
               lua_getfield(L, -1, #name);                                  \
               auto result = pull_condition_event_##name##_from_lua(L, -1); \
               lua_pop(L, 1);                                               \
               if (result.has_value())                                      \
                  ep.name = result.value();                                 \
               else {                                                       \
                  auto error = std::format("problem with `" #name "`: {}", result.error()); \
                  return std::unexpected(error);                            \
               }                                                            \
            }
         CASE(function)
         CASE(member)
         CASE(form)
         #undef CASE
      } else {
         for (size_t i = 0; i < 2; ++i) {
            lua_geti(L, -1, i + 1);
            auto result = pull_condition_parameter_from_lua(L, -1, self, working, i);
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