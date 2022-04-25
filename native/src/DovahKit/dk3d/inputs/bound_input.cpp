#include "bound_input.h"

namespace DK3D::inputs {
   /*static*/ bound_input bound_input::from_key(QChar c, button_press_type mod) {
      bound_input result;
      result.button.key        = c;
      result.button.press_type = mod;
      return result;
   }
   /*static*/ bound_input bound_input::from_mouse_button(Qt::MouseButton b, button_press_type mod) {
      bound_input result;
      result.button.mouse      = b;
      result.button.press_type = mod;
      return result;
   }
   /*static*/ bound_input bound_input::from_xinput_button(xinput_button b, button_press_type mod) {
      bound_input result;
      result.button.gamepad    = b;
      result.button.press_type = mod;
      return result;
   }
}