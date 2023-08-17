#pragma once
#include <string>
#include <string_view>

namespace cobb {
   constexpr void replace_all(std::string& str, std::string_view subject, std::string_view target) {
      if (target.size() > subject.size()) { // would a replacement make the string larger?
         size_t count = 0;
         {
            size_t offset = 0;
            while ((offset = str.find(subject, offset)) != std::string::npos)
               ++count;
         }
         if (!count)
            return;
         //
         // if so, pre-allocate room for all replacements:
         //
         size_t diff = (target.size() - subject.size()) * count;
         str.reserve(str.size() + diff);
      }

      size_t offset = 0;
      while ((offset = str.find(subject, offset)) != std::string::npos) {
         str.replace(offset, subject.size(), target);
         offset += target.size();
      }
   }
}