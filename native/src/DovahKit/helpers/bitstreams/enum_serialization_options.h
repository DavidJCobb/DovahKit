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
}