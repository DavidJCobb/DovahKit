#include "./stricontains_ascii.h"
#include <string> // std::string::npos
#include "./strieq_ascii.h"
#include "./strifind_ascii.h"

namespace cobb {
   extern bool stricontains_ascii(const std::string_view haystack, const std::string_view needle) {
      return strifind_ascii(haystack, needle) != std::string::npos;
   }
}