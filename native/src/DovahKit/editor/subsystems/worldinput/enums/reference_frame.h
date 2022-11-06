#pragma once

namespace dovahkit::subsystems::worldinput {
   enum class reference_frame {
      current = -1,
      //
      local  = 0,
      world  = 1,
      camera = 2,
   };
}
