#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../../bs_hash.h"
#include "./packed_file_info.h"

namespace dovah::bsa {
   struct packed_folder_info {
      bs_hash     hash;
      std::string name; // present only if the BSA uses the (include_directory_names) flag
      uint64_t    offset  = 0; // is uint32_t in Classic and a uint64_t in Special
      uint32_t    padding = 0; // Special adds a uint64_t to the end of the struct, so the legacy offset field becomes padding

      std::vector<packed_file_info> files; // must be sorted by file hash

      constexpr const packed_file_info* lookup_file_info(const bs_hash& file_hash) const noexcept;
      constexpr const packed_file_info* lookup_file_info(const bs_hash& file_hash, const std::string& filename) const noexcept;
      
      // if string args are non-empty and the archive contains strings, then args are used to verify hash correctness
      constexpr const packed_file_info* find_file(const bs_hash& file_hash, const std::string& file_name) const noexcept;
   };
}

#include "./packed_folder_info.inl"