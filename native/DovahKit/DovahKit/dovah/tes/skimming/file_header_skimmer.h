#pragma once
#include <bit>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include "../file_header_flags.h"

namespace dovah::tes {
   class file_header_skimmer {
      public:
         struct {
            std::vector<std::string> dependencies;
            std::endian              endianness   = std::endian::little;
            float                    file_version = 0;
            file_header_flags        flags;
            uint32_t                 record_and_group_count = 0;
         } results;

         void skim(const std::filesystem::path&);
   };
}