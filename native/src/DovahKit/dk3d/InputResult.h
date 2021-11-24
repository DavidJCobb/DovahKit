#pragma once
#include "BoundInput.h"
#include "chrono.h"

namespace DK3D {
   enum class KeyReleaseType {
      None,
      Tap,
      Hold,
   };

   struct BoundInput;
   struct KeyDownState;

   struct InputResult {
      enum class Type {
         None,
         Boolean,
         Scalar,
         Vector,
      };
      //
      Type  type = Type::None;
      float x    = 0;
      float y    = 0;
      BooleanInputMod bool_mod = BooleanInputMod::Tap;
      bool while_has_changed = false;

      bool active() const;

      static InputResult for_boolean_input(timestamp_t now, const BoundInput& bind, const KeyDownState&);
   };
}