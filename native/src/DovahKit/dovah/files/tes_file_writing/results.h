#pragma once
#include <string>

namespace dovah::tes_file_writing {
   class write_results {
      public:
         std::string filename;
         bool        saved_to_temporary_file = false;
   };
}