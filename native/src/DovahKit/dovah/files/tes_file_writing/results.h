#pragma once
#include <cstdint>
#include <vector>
#include "../common.h"
#include "../../core.h"
#include "../../detailed_notice.h"

namespace dovah::tes_file_writing {
   class write_results {
      public:
         detailed_notice error;
         std::vector<detailed_notice> warnings;
   };
}