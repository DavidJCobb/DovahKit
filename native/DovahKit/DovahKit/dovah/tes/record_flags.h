#pragma once
#include <cstdint>
#include "lu/enums/enum_flags.h"

namespace dovah::tes {
   enum class record_flag : uint32_t {
      deleted    =  5,
      persistent = 10,
      ignored    = 12,
      partial    = 14,
      compressed = 18,
   };

   using record_flags = lu::enum_flags<record_flag, 32>;
   static_assert(sizeof(record_flags) == 4);
}