#pragma once
#include <string_view>

namespace cobb {
   // Check if `haystack` contains `needle`, treating ASCII letters in each as case-insensitive;
   extern bool stricontains_ascii(const std::string_view haystack, const std::string_view needle);
}