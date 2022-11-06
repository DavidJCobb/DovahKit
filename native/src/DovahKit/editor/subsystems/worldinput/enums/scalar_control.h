#pragma once

namespace dovahkit::subsystems::worldinput {
   // A physical input that can produce one scalar value, e.g. a joystick trigger or a single axis on a joystick or mouse movement.
   enum class scalar_control {
      none,
      mouse_move,
      xinput_ls,
      xinput_rs,
      xinput_lt,
      xinput_rt,
   };
}