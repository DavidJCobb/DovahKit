#pragma once
#include "lu/enums/enum_flags.h"

namespace dovah::tes {
   enum class file_header_flag {
      master            = 0,
      localized_strings = 7,
      light             = 9,
   };
   using file_header_flags = lu::enum_flags<file_header_flag, 32>;
}