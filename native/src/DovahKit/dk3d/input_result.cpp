#include "input_result.h"
#include "button_state.h"
#include "inputs/bound_input.h"

namespace DK3D {
   bool input_result::active() const {
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
   bool input_result::is_button_release() const {
      return (this->type == control_type::none) && this->button.changed;
   }

   /*static*/ input_result input_result::for_button_input(timestamp_t now, const inputs::bound_input& bind, const button_state& kds) {
      bool is_while = bind.button.press_type == button_press_type::while_down;
      //
      input_result result;
      if (kds.is_down) {
         result.type = control_type::button;
         result.button.press_type = button_press_type::while_down;
         result.button.down_when  = kds.down_when;
         if (is_while) {
            if (kds.down_when == now) {
               //
               // "While" bindings may need to know when the key has just gone down.
               //
               result.button.changed = true;
            }
         }
      } else {
         result.type = control_type::button;
         switch (kds.released) {
            using _ = button_release_type;
            case _::tap:
               result.button.press_type = button_press_type::tap;
               break;
            case _::hold:
               result.button.press_type = button_press_type::hold;
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
      if (result.button.press_type != bind.button.press_type) {
         //
         // This particular action isn't what the binding was mapped to. Give a "none" 
         // result.
         //
         result.type = control_type::none;
         if (is_while && kds.released != button_release_type::none) {
            //
            // "While" bindings may still need to know when the key is released.
            //
            result.button.press_type = button_press_type::while_down;
            result.button.changed    = true;
         }
      }
      return result;
   }
   /*static*/ input_result input_result::for_button_release() {
      input_result out;
      out.button.press_type = button_press_type::while_down;
      out.button.changed    = true;
      return out;
   }
}