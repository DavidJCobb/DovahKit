#include "InputResult.h"
#include "KeyDownState.h"
#include "inputs/bound_input.h"

namespace DK3D {
   bool InputResult::active() const {
      switch (this->type) {
         using _ = control_type;
         case _::none:
            return false;
         case _::button:
            return true;
         case _::scalar:
            return this->x;
         case _::vector:
            return this->x || this->y;
      }
      return false;
   }

   /*static*/ InputResult InputResult::for_boolean_input(timestamp_t now, const inputs::bound_input& bind, const KeyDownState& kds) {
      bool is_while = bind.button.press_type == button_press_type::while_down;
      //
      InputResult result;
      if (kds.is_down) {
         result.type       = control_type::button;
         result.press_type = button_press_type::while_down;
         if (is_while) {
            if (kds.down_when == now) {
               //
               // "While" bindings may need to know when the key has just gone down.
               //
               result.while_has_changed = true;
            }
         }
      } else {
         result.type = control_type::button;
         switch (kds.released) {
            using _ = KeyReleaseType;
            case _::Tap:
               result.press_type = button_press_type::tap;
               break;
            case _::Hold:
               result.press_type = button_press_type::hold;
               break;
            default:
               result.type = control_type::none;
               break;
         }
      }
      //
      // Now that we know whether the key is down, how the key was released, and so on, 
      // let's check which of those the binding in question actually wanted.
      //
      if (result.press_type != bind.button.press_type) {
         //
         // This particular action isn't what the binding was mapped to. Give a "none" 
         // result.
         //
         result.type = control_type::none;
         if (is_while && kds.released != KeyReleaseType::None)
            //
            // "While" bindings may still need to know when the key is released.
            //
            result.while_has_changed = true;
      }
      return result;
   }
}