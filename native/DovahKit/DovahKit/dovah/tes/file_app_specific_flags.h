#pragma once
#include "lu/enums/enum_flags.h"

namespace dovah::tes {
   enum class file_app_specific_flag {
      is_hardcoded_dummy,
      is_none_stub_dummy,
      file_extension_forces_light,
      file_extension_forces_master,
      is_opposite_endianness,
   };

   using file_app_specific_flags = lu::enum_flags<file_app_specific_flag, 16>;
}