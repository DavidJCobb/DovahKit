#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include "lu/enums/enum_flags.h"
#include "lu/integral_character_code.h"
namespace dovah {
   class managed_localized_string_use;
   class managed_form_use;
}

namespace dovah::tes::serialization {
   template<typename T>
   struct serialized_bytecount_of;

   template<typename T>
      requires std::is_integral_v<T>
   struct serialized_bytecount_of<T> : public std::integral_constant<sizeof(T)> {};

   template<lu::enum_flags_type T>
   struct serialized_bytecount_of<T> : public serialized_bytecount_of<typename T::mask_type> {};

   template<lu::integral_character_code_type T>
   struct serialized_bytecount_of<T> : public serialized_bytecount_of<typename T::integral_type> {};

   template<>
   struct serialized_bytecount_of<managed_form_use> : public serialized_bytecount_of<uint32_t> {};

   template<>
   struct serialized_bytecount_of<managed_localized_string_use> : public serialized_bytecount_of<uint32_t> {};

   //
   // containers:
   //

   template<typename ValueType, size_t Size>
   struct serialized_bytecount_of<std::array<ValueType, Size>> : public std::integral_constant<(serialized_bytecount_of<ValueType>::value * Size)> {};
}
