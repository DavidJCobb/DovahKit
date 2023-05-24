#pragma once
#include <array>
#include <concepts>
#include <type_traits>

namespace cobb::bitstreams {
   template<typename Enum> requires std::is_enum_v<Enum>
   struct enum_serialization_options {
      enum_serialization_options() = delete;

      using value_type = Enum;
   };

   template<typename Enum>
   concept enum_has_explicit_bitcount_override = requires {
      requires std::is_enum_v<Enum>;
      { enum_serialization_options<Enum>::bitcount } -> std::convertible_to<size_t>;
   };

   template<typename Enum> requires std::is_enum_v<Enum>
   constexpr const size_t enum_bitcount = []() {
      if constexpr (enum_has_explicit_bitcount_override<Enum>) {
         return enum_serialization_options<Enum>::bitcount;
      }
      return sizeof(Enum) * 8;
   }();
}