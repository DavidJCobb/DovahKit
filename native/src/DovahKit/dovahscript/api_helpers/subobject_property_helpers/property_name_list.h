#pragma once
#include "./property_definition.h"
#include <array>
#include <string_view>
#include <tuple>
#include <type_traits>
#include "helpers/tuples/for_each_value.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<const auto& PropertiesTuple>
   constexpr const auto property_name_list = []() {
      std::array<std::string_view, std::tuple_size_v<std::decay_t<decltype(PropertiesTuple)>>> names = {};
      {
         size_t i = 0;
         cobb::tuples::for_each_value(PropertiesTuple, [&names, &i](const auto& definition) {
            names[i] = definition.name;
            ++i;
         });
      }
      return names;
   }();
}