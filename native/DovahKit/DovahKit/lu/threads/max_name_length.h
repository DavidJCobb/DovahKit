#pragma once
#include <limits>

namespace lu::threads {
   constexpr const size_t max_name_length =
      #if _MSC_VER
         std::numeric_limits<size_t>::max()
      #elif __GLIBC__ > 2 && _GLIBC_MINOR__ > 6
         15 // limit is 16 including the null terminator
      #else
         0
      #endif
   ;
}