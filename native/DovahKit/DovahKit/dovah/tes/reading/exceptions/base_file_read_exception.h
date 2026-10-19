#pragma once
#include <stdexcept>
namespace dovah::tes {
   class file;
}

namespace dovah::tes {
   class base_file_read_exception : public std::exception {
      public:
         base_file_read_exception(const file& f) : affected_file(f) {}

      public:
         const file& affected_file;
   };
}
