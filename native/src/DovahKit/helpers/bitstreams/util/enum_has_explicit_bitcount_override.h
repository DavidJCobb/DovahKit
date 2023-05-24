#pragma once
#include <concepts>
#include <type_traits>
#include "../enum_serialization_options.h"

namespace cobb::bitstreams::util {
   template<typename Enum>
   concept enum_has_explicit_bitcount_override = requires {
      requires std::is_enum_v<Enum>;
      { enum_serialization_options<Enum>::bitcount } -> std::convertible_to<size_t>;
   };
}