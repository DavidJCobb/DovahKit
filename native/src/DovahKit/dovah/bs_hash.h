#pragma once
#include <cstdint>

namespace dovah {
   union bs_hash {
      uint8_t  bytes[8];
      uint32_t dwords[2];
      uint64_t value = 0;
      
      constexpr bs_hash() {}
      constexpr bs_hash(uint64_t v) : value(v) {}
      bs_hash(const char* filename_or_folder_path, const char* extension); // NOTE: folder paths should not have redundant, leading, or trailing slashes, nor the leading "data" folder. example: `sound\voice\skyrim.esm\maleuniqueesbern`
      
      constexpr operator uint64_t() const noexcept { return this->value; }
   };
}