#pragma once
#include <cstdint>
#include <vector>
#include "../common.h"
#include "../../core.h"
#include "../../detailed_notice.h"

namespace dovah::tes_file_reading {
   class read_results {
      public:
         detailed_notice error;
         std::vector<detailed_notice> warnings;

         read_results();

         detailed_notice& add_warning() noexcept;
         void add_warning(detailed_notice&) noexcept;
   };
}