#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "../common.h"
#include "../../core.h"
#include "../../detailed_notice.h"

namespace dovah::notices {
   class base_file_load_warning;
}

namespace dovah::tes_file_reading {
   class read_results {
      public:
         detailed_notice error;
         std::vector<detailed_notice> warnings;
         std::vector<std::unique_ptr<notices::base_file_load_warning>> warnings_ex;

         read_results();

         detailed_notice& add_warning() noexcept;
         void add_warning(detailed_notice&) noexcept;
         void add_warning(const notices::base_file_load_warning&);

         constexpr bool failed() const {
            return error.is_defined();
         }
   };
}