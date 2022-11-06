#pragma once
#include "helpers/enum_flags.h"

namespace dovahkit::subsystems::worldinput {
   enum class editor_mode {
      object,
      landscape,
      navmesh,
   };

   using editor_mode_set = cobb::enum_flags<editor_mode, 3>;
   inline constexpr editor_mode_set all_editor_modes = editor_mode_set::with_all_set();
}
