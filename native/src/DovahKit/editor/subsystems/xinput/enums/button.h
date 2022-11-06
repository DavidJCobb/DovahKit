#pragma once
#include <cstdint>
#include <limits>
#include <windows.h> // required or the XInput header will make MSVC choke
#include <xinput.h>
#include "helpers/intrusive_windows_defines.h"

namespace dovahkit::subsystems::xinput {
   enum class button {
      none = 0,

      a           = XINPUT_GAMEPAD_A,
      b           = XINPUT_GAMEPAD_B,
      x           = XINPUT_GAMEPAD_X,
      y           = XINPUT_GAMEPAD_Y,
      start       = XINPUT_GAMEPAD_START,
      back        = XINPUT_GAMEPAD_BACK,
      d_pad_up    = XINPUT_GAMEPAD_DPAD_UP,
      d_pad_down  = XINPUT_GAMEPAD_DPAD_DOWN,
      d_pad_left  = XINPUT_GAMEPAD_DPAD_LEFT,
      d_pad_right = XINPUT_GAMEPAD_DPAD_RIGHT,
      
      stick_click_left  = XINPUT_GAMEPAD_LEFT_THUMB,
      ls                = stick_click_left,
      stick_click_right = XINPUT_GAMEPAD_RIGHT_THUMB,
      rs                = stick_click_right,
      
      bumper_left  = XINPUT_GAMEPAD_LEFT_SHOULDER,
      lb           = bumper_left,
      bumper_right = XINPUT_GAMEPAD_RIGHT_SHOULDER,
      rb           = bumper_right,

      //
      // Things that are not buttons that we may nonetheless wish to treat like buttons:
      //
      pseudo_buttons_start = std::numeric_limits<uint16_t>::max(),
      
      trigger_left  = pseudo_buttons_start + 1,
      lt            = trigger_left,
      trigger_right = pseudo_buttons_start + 2,
      rt            = trigger_right,
   };
}