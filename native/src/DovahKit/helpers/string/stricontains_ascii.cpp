#include "./stricontains_ascii.h"
#include <string> // std::string::npos
#include "./strieq_ascii.h"

namespace cobb {
   extern bool stricontains_ascii(const std::string_view haystack, const std::string_view needle) {
      if (!needle.size())
         return true;

      char desired[2] = { needle[0], needle[0] };
      if (char c = desired[0]; c >= 'A' && c <= 'Z')
         desired[1] = c + 0x20;

      size_t i = haystack.find_first_of(desired, 2);
      if (i != std::string::npos) {
         if (i + needle.size() > haystack.size())
            return false;
         
         auto subject = std::string_view(haystack).substr(i);
         if (!cobb::strieq_ascii(subject, needle))
            return false;
      }
      return true;
   }
}