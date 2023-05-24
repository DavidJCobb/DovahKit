#pragma once
#include <limits>
#include <type_traits>
#include "../enum_serialization_options.h"
#include "./enum_has_contiguous_valid_values.h"

namespace cobb::bitstreams::util {
   template<typename T> requires std::is_enum_v<T>
   struct enum_valid_value_range;

   template<typename T> requires enum_has_contiguous_valid_values<T>
   struct enum_valid_value_range<T> {
      using underlying_type = std::underlying_type_t<T>;

      static constexpr const underlying_type min = []() {
         underlying_type v = std::numeric_limits<underlying_type>::max();

         const auto& list = enum_serialization_options<Enum>::valid_values;
         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] < v)
               v = list[i];

         return v;
      }();
      static constexpr const underlying_type max = []() {
         underlying_type v = std::numeric_limits<underlying_type>::lowest();

         const auto& list = enum_serialization_options<Enum>::valid_values;
         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] > v)
               v = list[i];

         return v;
      }();
   };
}