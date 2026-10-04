#include "./strifind_ascii.h"
#include <string> // std::string::npos
#include "./strieq_ascii.h"

namespace cobb {
   extern size_t strifind_ascii(const std::string_view haystack, const std::string_view needle, size_t offset) {
      if (offset >= needle.size())
         return std::string::npos;

      char desired[2] = { needle[0], needle[0] };
      if (char c = desired[0]; c >= 'A' && c <= 'Z')
         desired[1] = c + 0x20;

      size_t i = haystack.find_first_of(desired, offset, 2);
      while (i != std::string::npos) {
         if (i + needle.size() > haystack.size())
            return std::string::npos;
         auto subject = haystack.substr(i, needle.size());
         if (cobb::strieq_ascii(subject, needle))
            return i;
         i = haystack.find_first_of(desired, i + 1, 2);
      }
      return std::string::npos;
   }
}