#pragma once
#include <cstdint>
#include <string>
#include "../../bs_hash.h"

namespace dovah::bsa {
   struct packed_file_info {
      bs_hash     hash;
      std::string name; // present only if the BSA uses the (include_filenames) flag
      uint32_t    size_and_flags = 0; // size of the data embedded in the BSA, i.e. the compressed data if compression is in use
      uint32_t    offset         = 0; // offset of: the embedded filename, if any, with a one-byte length prefix and no null terminator; the uncompressed size (if the file is compressed); and then the file data
      bool        corrupt = false; // set if the file claims to be too large to fit in the BSA, such that reading it would read past the BSA's end

      constexpr uint32_t size() const noexcept { return this->size_and_flags & ~0xC0000000; }
      constexpr bool invalidated() const noexcept { return (this->size_and_flags & 0x80000000) != 0; } // this flag should only be set at run-time
      constexpr bool non_default_compression() const noexcept { return (this->size_and_flags & 0x40000000) != 0; }
   };
}
