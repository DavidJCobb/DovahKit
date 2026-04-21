#pragma once
#include <array>
#include <string_view>
#include <type_traits>
#include <utility>
#include "helpers/type_traits/is_std_array.h"
#include "helpers/type_traits/is_std_pair.h"
#include "../common_lambdas.h"
#include "../property_definition.h"
#include "../utils/is_accessor_for_type.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   namespace impl::enumeration_property {
      template<typename T>
      concept is_value_to_name_map = requires {
         requires cobb::is_std_array<T>;
         requires cobb::is_std_pair<std::tuple_element_t<0, T>>;
         typename std::tuple_element_t<0, T>::first_type;
         typename std::tuple_element_t<0, T>::second_type;
         requires std::is_enum_v<typename std::tuple_element_t<0, T>::first_type>;
         requires std::is_same_v<std::string_view, typename std::tuple_element_t<0, T>::second_type>;
      };
   }

   template<const auto& ValuesToNames>
      requires impl::enumeration_property::is_value_to_name_map<std::decay_t<decltype(ValuesToNames)>>
   struct enumeration_property {
      enumeration_property() = delete;

      using mapping_type    = std::decay_t<decltype(ValuesToNames)>;
      using enumeration_type = typename std::tuple_element_t<0, mapping_type>::first_type;

      static std::string_view check_lua_value(lua_State* L, int pos) {
         if (!lua_isstring(L, pos))
            return "string expected";
         std::string_view v = lua_tostring(L, pos);
         for (const auto& pair : ValuesToNames)
            if (v == pair.second)
               return {};
         return "unrecognized value";
      }
      static enumeration_type pull_lua_value(lua_State* L, int pos) {
         std::string_view v = lua_tostring(L, pos);
         for (const auto& pair : ValuesToNames)
            if (v == pair.second)
               return pair.first;
         std::unreachable();
      }
      static void push_lua_value(lua_State* L, const enumeration_type& v) {
         for (const auto& pair : ValuesToNames) {
            if (v == pair.first) {
               lua_pushlstring(L, pair.second.data(), pair.second.size());
               return;
            }
         }
         lua_pushnil(L);
      }
      
      template<typename AccessFunc>
         requires utils::is_accessor_for_type<enumeration_type, AccessFunc>
      static consteval auto define(std::string_view name, AccessFunc a) {
         return property_definition{
            .name   = name,
            .access = a,
            .check  = &check_lua_value,
            .pull   = &pull_lua_value,
            .push   = &push_lua_value,
         };
      }

      template<typename AccessFunc>
         requires utils::is_accessor_for_type<enumeration_type, AccessFunc>
      static consteval auto define(std::string_view name, AccessFunc a, enumeration_type dv) {
         return property_definition{
            .name   = name,
            .access = a,
            .check  = &check_lua_value,
            .pull   = &pull_lua_value,
            .push   = &push_lua_value,
            .default_value = dv,
         };
      }
   };
}