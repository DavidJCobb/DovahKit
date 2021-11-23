#pragma once
#include "chrono.h"
#include "BoundInput.h"

namespace DK3D {
   struct InputResult {
      enum class Type {
         None,
         Tap,
         Hold,
         While,
         Scalar,
         Vector,
      };
      //
      Type  type = Type::None;
      float x    = 0;
      float y    = 0;
      bool  while_has_changed = false;

      bool active() const;

      static InputResult from_boolean(BooleanInputMod m);
   };

   class BoundInputState {
      public:
         BoundInput  input;
         timestamp_t start = zero_timestamp;
         bool ignore = false;
         //
         InputResult last_result;

         BoundInputState() {}
         BoundInputState(const BoundInput& a) : input(a) {}

         void ignore_if_down(timestamp_t now);
         InputResult update(timestamp_t now);
   };
}