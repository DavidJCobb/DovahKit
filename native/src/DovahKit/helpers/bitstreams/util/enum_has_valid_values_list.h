#pragma once
#include <type_traits>
#include "../enum_serialization_options.h"

namespace cobb::bitstreams::util {
   template<typename Enum>
   concept enum_has_valid_values_list = requires {
      requires std::is_enum_v<Enum>;
      typename enum_serialization_options<Enum>::value_type;
      requires std::is_same_v<Enum, typename enum_serialization_options<Enum>::value_type>;
      { enum_serialization_options<Enum>::valid_values } -> std::same_as<const std::array<Enum, std::tuple_size<decltype(enum_serialization_options<Enum>::valid_values)>::value>&>;
   };
}