#pragma once
#include <cstdint>
#include <type_traits>
#include "./entry_flag_underlying_type.h"

namespace dovah {
   class form_stub;
}

namespace dovah::use_info {
   struct entry {
      public:
         form_stub* other    = nullptr;
         uint32_t   refcount = 0;
         entry_flag_underlying_type flags = 0;
   };
}