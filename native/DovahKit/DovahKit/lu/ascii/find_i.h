#pragma once
#include <string> // std::string::npos
#include <string_view>
#include "./equals_i.h"

namespace lu::ascii {
   // Find `needle` in `haystack`, treating ASCII letters in each as case-insensitive. 
   // Return `std::string::npos` if `needle` is not present.
   constexpr size_t find_i(const std::string_view haystack, const std::string_view needle, const size_t from) {
      if (from >= haystack.size())
         return std::string::npos;
      if (needle.empty())
         return from;

      char desired[3] = { needle[0], needle[0], '\0' };
      if (char c = desired[0]; c >= 'A' && c <= 'Z')
         desired[1] = c + 0x20;
      else if (c >= 'a' && c <= 'z')
         desired[1] = c - 0x20;

      size_t i = haystack.find_first_of(desired, from, 2);
      while (i != std::string_view::npos) {
         if (i + needle.size() > haystack.size())
            return false;

         auto subject = haystack.substr(i, needle.size());
         if (equals_i(subject, needle))
            return i;

         i = haystack.find_first_of(desired, i + 1, 2);
      }
      return std::string::npos;
   }
}