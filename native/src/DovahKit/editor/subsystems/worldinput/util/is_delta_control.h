#pragma once
#include "../enums/range_input_control.h"

namespace dovahkit::subsystems::worldinput::util {
   constexpr bool is_delta_control(range_input_control c) {
      switch (c) {
         case range_input_control::mouse_move:
            return true;
      }
      return false;
   }
}