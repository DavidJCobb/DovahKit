#pragma once
#include "./is_property_tuple_type.h"
#include "./property_definition.h"
#include "./property_name_list.h"
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include "helpers/tuples/for_each_nttp_value.h"
#include "lua.h"

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

         std::string_view first_seen_expando_name;
         {
            lua_pushnil(L); // first key
            while (lua_next(L, table_pos) != 0) {
               lua_pushvalue(L, -2); // copy the key so we don't modify its type in-place
               std::string_view key = lua_tostring(L, -1);
               lua_pop(L, 1);
               bool found = false;
               for (const auto& allowed : property_names) {
                  if (key == allowed) {
                     found = true;
                     break;
                  }
               }
               lua_pop(L, 1);

               if (!found) {
                  first_seen_expando_name = key;
                  break;
               }
            }
         }
         if (!first_seen_expando_name.empty()) {
            return std::format("table contains one or more unexpected keys (first seen: `%s`)", first_seen_expando_name);
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
                  error_message = std::format("table is missing a value for key `%s`", Definition.name);
               }
            } else {
               const auto field_error = Definition.check(L, -1);
               if (!field_error.empty())
                  error_message = std::format("table has an invalid value for key `%s`: %s", Definition.name, field_error);
            }
            lua_pop(L, 1);
         }
      >(L, table_pos, error_message);
      return error_message;
   }
}