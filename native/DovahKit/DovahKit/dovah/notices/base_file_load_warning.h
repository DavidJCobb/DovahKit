#pragma once
#include <string>

namespace dovah::notices {
   class base_file_load_warning {
      public:
         virtual ~base_file_load_warning() {}
      public:
         constexpr base_file_load_warning(const std::string& source_file) : source_file(source_file) {}

      public:
         std::string source_file;
   };
}
