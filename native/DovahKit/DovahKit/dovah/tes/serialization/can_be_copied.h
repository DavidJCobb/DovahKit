#pragma once
#include <array>
#include <type_traits>
#include "lu/enums/enum_flags.h"
#include "lu/integral_character_code.h"
namespace dovah {
   class managed_localized_string_use;
   class managed_form_use;
}

namespace dovah::tes::serialization {
   // True if the type can simply be `memcpy`'d with no further processing. False if 
   // any further processing is needed (e.g. form IDs, localized string IDs).
   template<typename T>
   struct can_be_copied : public std::bool_constant<false> {};

   template<typename T>
      requires std::is_integral_v<T>
   struct can_be_copied<T> : public std::bool_constant<true> {};

   template<typename T, size_t S>
   struct can_be_copied<std::array<T, S>> : public can_be_copied<T> {};

   template<lu::enum_flags_type T>
   struct can_be_copied<T> : public std::bool_constant<true> {};

   template<lu::integral_character_code_type T>
   struct can_be_copied<T> : public std::bool_constant<true> {};

   // explicit "no":
   template<>
   struct can_be_copied<managed_localized_string_use> : public std::bool_constant<false> {};
   template<>
   struct can_be_copied<managed_form_use> : public std::bool_constant<false> {};
}
