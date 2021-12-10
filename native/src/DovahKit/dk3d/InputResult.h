#pragma once
#include "BoundInput.h"
#include "chrono.h"
#include "enums/button_press_type.h"
#include "enums/control_type.h"

namespace DK3D {
   enum class KeyReleaseType {
      None,
      Tap,
      Hold,
   };

   struct KeyDownState;
   namespace inputs {
      struct bound_input;
   }

   struct InputResult {
      enum class Type {
         None,
         Boolean,
         Scalar,
         Vector,
      };
      //
      float x = 0;
      float y = 0;
      control_type      type       = control_type::none;
      button_press_type press_type = button_press_type::tap;
      bool while_has_changed = false;

      bool active() const;

      static InputResult for_boolean_input(timestamp_t now, const inputs::bound_input& bind, const KeyDownState&);
   };
}