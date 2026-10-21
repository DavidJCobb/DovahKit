#pragma once
#include <cstdint>
#include "lu/enums/enum_flags.h"

namespace dovah::bsa {
   enum class archive_filetype_flag {
      meshes,
      textures,
      menus,
      sounds,
      voices,
      shaders,
      trees,
      fonts,
      miscellaneous,
   };

   using archive_filetype_flags = lu::enum_flags<archive_filetype_flag, 32>;
}