#pragma once

namespace dovahkit::subsystems::worldinput {
   // A physical input that can produce two scalar values, e.g. the movement of a joystick or the mouse.
   enum class vector_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
   };
}