#pragma once
#include <type_traits>
#include "./bitpacked_primitive_type.h"

namespace cobb::bitstreams::util {
   template<typename T> concept memcpyable_primitive_type = []() -> bool {
      if constexpr (!std::is_trivially_copyable_v<T>)
         return false;

      if constexpr (!bitstreamable_primitive<T>)
         return false;

      if constexpr (bitpacked_primitive_type<T>)
         return false;

      return true;
   }();
}