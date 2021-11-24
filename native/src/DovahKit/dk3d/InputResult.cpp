#include "InputResult.h"
#include "BoundInput.h"
#include "KeyDownState.h"

namespace DK3D {
   bool InputResult::active() const {
      switch (this->type) {
         case Type::None:
            return false;
         case Type::Boolean:
            return true;
         case Type::Scalar:
            return this->x;
         case Type::Vector:
            return this->x || this->y;
      }
      return false;
   }

   /*static*/ InputResult InputResult::for_boolean_input(timestamp_t now, const BoundInput& bind, const KeyDownState& kds) {
      bool is_while = bind.boolean.type == BooleanInputMod::While;
      //
      InputResult result;
      if (kds.is_down) {
         result.type     = InputResult::Type::Boolean;
         result.bool_mod = BooleanInputMod::While;
         if (is_while) {
            if (kds.down_when == now) {
               //
               // "While" bindings may need to know when the key has just gone down.
               //
               result.while_has_changed = true;
            }
         }
      } else {
         result.type = InputResult::Type::Boolean;
         switch (kds.released) {
            using _ = KeyReleaseType;
            case _::Tap:
               result.bool_mod = BooleanInputMod::Tap;
               break;
            case _::Hold:
               result.bool_mod = BooleanInputMod::Hold;
               break;
            default:
               result.type = InputResult::Type::None;
               break;
         }
      }
      //
      // Now that we know whether the key is down, how the key was released, and so on, 
      // let's check which of those the binding in question actually wanted.
      //
      if (result.bool_mod != bind.boolean.type) {
         //
         // This particular action isn't what the binding was mapped to. Give a "none" 
         // result.
         //
         result.type = InputResult::Type::None;
         if (is_while && kds.released != KeyReleaseType::None)
            //
            // "While" bindings may still need to know when the key is released.
            //
            result.while_has_changed = true;
      }
      return result;
   }
}