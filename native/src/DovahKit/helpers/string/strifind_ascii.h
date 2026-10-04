#pragma once
#include <string_view>

namespace cobb {
   // Find `needle` in `haystack`, treating ASCII letters in each as case-insensitive.
   extern size_t strifind_ascii(const std::string_view haystack, const std::string_view needle, size_t offset = 0);
}