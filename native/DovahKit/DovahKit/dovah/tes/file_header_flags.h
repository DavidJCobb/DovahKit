#pragma once
#include "lu/enums/enum_flags.h"

namespace dovah::tes {
   enum class file_header_flag {
      master            = 0,
      is_little_endian  = 6, // game/CK clobbers this on load: if header signature is 'T' 'E' 'S' '4' when read as a char[4], this is set; else, cleared
      localized_strings = 7,
      light             = 9,
   };
   using file_header_flags = lu::enum_flags<file_header_flag, 32>;
}