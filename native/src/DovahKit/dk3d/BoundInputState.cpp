#include "BoundInputState.h"
#include "DK3DInputHandler.h"

namespace {
   double boolean_input_hold_threshold = 0.7;
}

namespace DK3D {
   bool InputResult::active() const {
      switch (this->type) {
         case Type::None:
            return false;
         case Type::Tap:
         case Type::Hold:
         case Type::While:
            return true;
         case Type::Scalar:
            return this->x;
         case Type::Vector:
            return this->x || this->y;
      }
      return false;
   }
   /*static*/ InputResult InputResult::from_boolean(BooleanInputMod m) {
      InputResult res;
      switch (m) {
         using _ = BooleanInputMod;
         case _::Tap:
            res.type = Type::Tap;
            break;
         case _::Hold:
            res.type = Type::Hold;
            break;
         case _::While:
            res.type = Type::While;
            break;
      }
      return res;
   }



   void BoundInputState::ignore_if_down(timestamp_t now) {
      if (!this->input.is_boolean())
         return;
      bool down = this->input.is_boolean_down();
      if (!down) {
         this->start = zero_timestamp;
         return;
      }
      if (this->start == zero_timestamp)
         this->start = now;
      this->ignore = true;
   }
   InputResult BoundInputState::update(timestamp_t now) {
      this->last_result = InputResult();
      if (this->input.is_boolean()) {
         bool down = this->input.is_boolean_down();
         if (this->ignore) {
            //
            // We want to ignore this held key, perhaps because the program 
            // lagged and it took weirdly long to poll for input state.
            //
            if (!down)
               //
               // Stop ignoring a held key once it's released.
               //
               this->ignore = false;
            return InputResult();
         }
         if (down) {
            bool changed = (this->start == zero_timestamp);
            if (this->start == zero_timestamp)
               this->start = now;
            if (this->input.boolean.type == BooleanInputMod::While) {
               this->last_result = InputResult::from_boolean(BooleanInputMod::While);
               if (changed)
                  this->last_result.while_has_changed = true;
               return this->last_result;
            }
            return InputResult();
         }
         //
         // Key is up.
         //
         if (this->start == zero_timestamp)
            return InputResult();
         //
         // Key was previously down, and has now been released.
         //
         auto elapsed = elapsed_time(this->start, now);
         this->start = zero_timestamp;
         if (this->input.boolean.type == BooleanInputMod::While) {
            auto res = InputResult();
            res.while_has_changed = true;
            this->last_result = res;
            return res;
         }
         bool wants_hold = this->input.boolean.type == BooleanInputMod::Hold;
         bool input_hold = (elapsed >= boolean_input_hold_threshold);
         if (wants_hold != input_hold)
            return InputResult();
         if (elapsed >= boolean_input_hold_threshold) {
            return (this->last_result = InputResult::from_boolean(BooleanInputMod::Hold));
         }
         return (this->last_result = InputResult::from_boolean(BooleanInputMod::Tap));
      }
      //
      // Not a boolean input.
      //
      auto& ih = DK3DInputHandler::get();
      switch (this->input.scalar.input) {
         using _ = ScalarControl;
         case _::None:
            break;
         default:
            {
               InputResult res;
               res.type = InputResult::Type::Scalar;
               res.x    = ih.scalarControlValue(this->input.scalar.input);
               res.y    = 0;
               this->last_result = res;
               return res;
            }
      }
      switch (this->input.vector.input) {
         using _ = VectorControl;
         case _::None:
            break;
         default:
            {
               auto pt = ih.vectorControlValue(this->input.vector.input);
               //
               InputResult res;
               res.type = InputResult::Type::Vector;
               res.x = pt.x();
               res.y = pt.y();
               this->last_result = res;
               return res;
            }
      }
      return InputResult();
   }
}