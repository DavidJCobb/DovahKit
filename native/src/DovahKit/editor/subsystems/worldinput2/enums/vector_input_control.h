#pragma once

namespace dovahkit::subsystems::worldinput2 {
   // A physical input control that can produce two scalar values, e.g. the movement of a joystick or the mouse.
   enum class vector_input_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
   };
}