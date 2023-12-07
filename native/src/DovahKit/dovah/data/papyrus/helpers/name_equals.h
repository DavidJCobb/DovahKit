#pragma once
#include <string_view>

namespace dovah::papyrus::helpers {
   constexpr bool name_equals(const std::string_view& a, const std::string_view& b) {
      size_t size = a.size();
      if (size != b.size())
         return false;
      for (size_t i = 0; i < size; ++i) {
         char u = a[i];
         char v = b[i];
         if (u == v)
            continue;
         //
         // Papyrus scriptnames are case-insensitive, but IIRC that's a (deliberate, I'm sure) consequence 
         // of them using Bethesda's string-table system, which enforces C locale case-insensitivity.
         //
         if (u >= 'a' && u <= 'z')
            u -= 32;
         else if (v >= 'a' && v <= 'z')
            v -= 32;
         if (u != v)
            return false;
      }
      return true;
   }
}