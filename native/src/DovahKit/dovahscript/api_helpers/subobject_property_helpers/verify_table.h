#pragma once
#include "./is_property_tuple_type.h"
#include "./property_definition.h"
#include "./property_name_list.h"
#include <format>
#include <string>
#include <type_traits>
#include "helpers/tuples/for_each_nttp_value.h"
#include "lua.h"
#include "../table_contains_expandos.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<typename Dst, bool AlwaysAllowNil, const auto& PropertiesTuple>
      requires is_property_tuple_type<std::decay_t<decltype(PropertiesTuple)>>
   std::string verify_table(lua_State* L, int table_pos) {
      switch (lua_type(L, table_pos)) {
         case LUA_TTABLE:
         case LUA_TUSERDATA:
            break;
         default:
            return "expected table or userdata";
      }

      // reject expandos
      {
         constexpr auto property_names = property_name_list<PropertiesTuple>;

         auto pair = table_contains_expandos(L, table_pos, property_names);
         if (pair.first) {
            if (!pair.second.empty()) {
               return std::format("table contains one or more unexpected keys (first seen: `{}`)", pair.second);
            }
            return std::format("table contains one or more unexpected keys");
         }
      }

      std::string error_message;
      cobb::tuples::for_each_nttp_value<
         PropertiesTuple,
         []<const auto& Definition>(lua_State* L, int table_pos, std::string& error_message) {
            if (!error_message.empty())
               return;
            lua_getfield(L, table_pos, Definition.name.data());
            if (lua_isnoneornil(L, -1)) {
               if constexpr (!AlwaysAllowNil && !Definition.default_value.has_value() && !Definition.treat_nil_as_unchanged) {
                  error_message = std::format("table is missing a value for key `{}`", Definition.name);
               }
            } else {
               const auto field_error = Definition.check(L, -1);
               if (!field_error.empty())
                  error_message = std::format("table has an invalid value for key `{}`: {}", Definition.name, field_error);
            }
            lua_pop(L, 1);
         }
      >(L, table_pos, error_message);
      return error_message;
   }
}