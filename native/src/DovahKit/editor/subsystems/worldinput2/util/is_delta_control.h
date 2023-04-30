#pragma once
#include "../enums/scalar_input_control.h"
#include "../enums/vector_input_control.h"

namespace dovahkit::subsystems::worldinput2::util {
   constexpr bool is_delta_control(scalar_input_control c) {
      switch (c) {
         case scalar_input_control::mouse_move:
         // TODO: if we implement mouse wheels, they'll be a delta scalar control, so we'd want a fallthrough case for that here
            return true;
      }
      return false;
   }
   constexpr bool is_delta_control(vector_input_control c) {
      switch (c) {
         case vector_input_control::mouse_move:
            return true;
      }
      return false;
   }
}