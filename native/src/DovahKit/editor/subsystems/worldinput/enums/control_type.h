#pragma once

namespace dovahkit::subsystems::worldinput {
   enum class control_type {
      none = -1,
      //
      button = 0,
      scalar = 1,
      vector = 2,
   };
}