#pragma once

namespace dovahkit::subsystems::worldinput2 {
   // A physical input control that can produce one scalar value, e.g. a joystick trigger or a single axis on a joystick or mouse movement.
   enum class scalar_input_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
      xinput_lt,
      xinput_rt,
   };
}