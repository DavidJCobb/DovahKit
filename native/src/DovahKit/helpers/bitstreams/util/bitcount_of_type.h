#pragma once
#include <type_traits>
#include "../bitstreamable_primitive.h"
#include "./enum_type_information.h"

namespace cobb::bitstreams::util {
   template<bitstreamable_primitive T>
   constexpr const size_t bitcount_of_type = []() -> size_t {
      if constexpr (std::is_same_v<T, bool>) {
         return 1;
      }
      if constexpr (std::is_enum_v<T>) {
         return enum_type_information<T>::default_bitcount;
      }
      return sizeof(T) * 8;
   }();
}