#pragma once
#include <bit>
#include <cstdint>
#include <string_view>

namespace dovah {
   union bs_hash {
      public:
         uint8_t  bytes[8];
         uint32_t dwords[2];
         uint64_t value = 0;
      
         constexpr bs_hash() {}
         constexpr bs_hash(uint64_t v) : value(v) {}
         constexpr bs_hash(std::string_view filename_or_folder_path, std::string_view extension) {
            *this = via_skyrim_algorithm(filename_or_folder_path, extension);
         }
      
         constexpr operator uint64_t() const noexcept { return this->value; }

         // Paths should not have redundant, leading, or trailing slashes, nor the "data/" prefix.
         static constexpr bs_hash via_skyrim_algorithm(std::string_view path, std::string_view extension);
   };
}

#include "./bs_hash.inl"