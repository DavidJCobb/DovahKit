#pragma once
#include <type_traits>
#include "../bitstreamable_primitive.h"
#include "../enum_serialization_options.h"
#include "./bitcount_of_type.h"

namespace cobb::bitstreams::util {
   template<typename T> concept bitpacked_primitive_type = []() -> bool {
      if constexpr (!bitstreamable_primitive<T>)
         return false;

      return (bitcount_of_type<T> % 8) != 0;
   }();
}