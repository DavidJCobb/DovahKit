#pragma once
#include <cstdint>

namespace dovah::bsa {
   enum class archive_version : uint32_t {
      oblivion          = 0x67,
      fallout_3         = 0x68,
      fallout_new_vegas = 0x68,
      skyrim_classic    = 0x68,
      skyrim_special    = 0x69,
   };
}
