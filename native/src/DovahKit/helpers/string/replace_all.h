#pragma once
#include <string>
#include <string_view>

namespace cobb {
   template<typename Subject> requires (std::is_same_v<Subject, std::string_view> || std::is_same_v<Subject, char>)
   constexpr void replace_all(std::string& str, Subject subject, std::string_view target) {
      size_t subject_size = 0;
      if constexpr (std::is_same_v<Subject, char>)
         subject_size = 1;
      else
         subject_size = subject.size();

      if (target.size() > subject_size) { // would a replacement make the string larger?
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
         size_t diff = (target.size() - subject_size) * count;
         str.reserve(str.size() + diff);
      }

      size_t offset = 0;
      while ((offset = str.find(subject, offset)) != std::string::npos) {
         str.replace(offset, subject_size, target);
         offset += target.size();
      }
   }
}