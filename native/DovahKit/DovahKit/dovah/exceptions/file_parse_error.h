#pragma once
#include <stdexcept>
namespace dovah::tes {
   class file;
}

namespace dovah::exceptions {
   class file_parse_error : public std::runtime_error {
      public:
         file_parse_error(
            const tes::file& affected_file
         ) :
            std::runtime_error("file parse error"),
            affected_file(affected_file)
         {}

      public:
         const tes::file& affected_file;
   };
}