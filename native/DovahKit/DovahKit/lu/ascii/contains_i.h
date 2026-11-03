#pragma once
#include "./find_i.h"

namespace lu::ascii {
   // Check if `haystack` contains `needle`, treating ASCII letters in each as case-insensitive.
   constexpr bool contains_i(const std::string_view haystack, const std::string_view needle) {
      return find_i(haystack, needle, 0) != std::string::npos;
   }
}