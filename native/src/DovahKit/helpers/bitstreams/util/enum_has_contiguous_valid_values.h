#pragma once
#include <type_traits>
#include "../enum_serialization_options.h"
#include "./enum_has_valid_values_list.h"

namespace cobb::bitstreams::util {
   template<typename Enum> requires std::is_enum_v<Enum>
   constexpr const bool enum_has_contiguous_valid_values = []() {
      if constexpr (enum_has_valid_values_list<Enum>) {
         using integral_type = std::underlying_type_t<Enum>;

         const auto& list = enum_serialization_options<Enum>::valid_values;
         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] != list[i - 1] + 1)
               return false;
      }
      return false;
   }();
}