#include "BoundInput.h"
#include <windows.h>
#include "../helpers/intrusive_windows_defines.h"

namespace {
   int qt_mouse_button_to_vk(Qt::MouseButton b) {
      switch (b) {
         using _ = Qt::MouseButton;
         case _::LeftButton:
            return VK_LBUTTON;
         case _::RightButton:
            return VK_RBUTTON;
         case _::MiddleButton:
            return VK_MBUTTON;
         case _::XButton1:
            return VK_XBUTTON1;
         case _::XButton2:
            return VK_XBUTTON2;
      }
      return 0;
   }
}

namespace DK3D {
   // TODO: we should have a singleton manage key states, so that we aren't calling 
   // GetAsyncKeyState multiple times per frame for the same key (given any partially 
   // overlapping binds). Ideally, the singleton could keep track of which keys are 
   // actually bound, and just update their states per frame. That same singleton 
   // would be needed to poll mouse movement over time.

   bool BoundInput::is_boolean_down() const {
      if (!this->boolean.key.empty()) {
         auto vk = this->boolean.key.native.vk;
         if (vk) {
            return (GetAsyncKeyState(vk) & 0x8000) != 0;
         }
         return false;
      }
      if (this->boolean.gamepad.button != XInputKey::None) {
         auto& xi = DKXInputSubsystem::get();
         if (xi.isGamepadConnected()) {
            const auto& s = xi.gamepadState();
            return s.isButtonDown(this->boolean.gamepad.button);
         }
         return false;
      }
      if (this->boolean.mouse.button != Qt::MouseButton::NoButton) {
         auto vk = qt_mouse_button_to_vk(this->boolean.mouse.button);
         if (vk) {
            return (GetAsyncKeyState(vk) & 0x8000) != 0;
         }
         return false;
      }
      return false;
   }

   /*static*/ BoundInput BoundInput::from_key(QChar c, BooleanInputMod mod) {
      BoundInput result;
      result.boolean.key  = c;
      result.boolean.type = mod;
      return result;
   }
   /*static*/ BoundInput BoundInput::from_xinput_button(XInputKey b, BooleanInputMod mod) {
      BoundInput result;
      result.boolean.gamepad.button = b;
      result.boolean.type = mod;
      return result;
   }
}