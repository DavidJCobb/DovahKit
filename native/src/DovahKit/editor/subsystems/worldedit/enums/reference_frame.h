#pragma once

namespace dovahkit::subsystems::worldedit {
   enum class reference_frame {
      current = -1,
      //
      local  = 0,
      world  = 1,
      camera = 2,
   };
}
