#include "XInputGamepadState.h"
#include "../helpers/unreachable.h"
#include "defaults.h"

namespace {
   constexpr std::array _indices_to_buttons = {
      DK3D::XInputGamepadState::Button::A,
      DK3D::XInputGamepadState::Button::B,
      DK3D::XInputGamepadState::Button::X,
      DK3D::XInputGamepadState::Button::Y,
      DK3D::XInputGamepadState::Button::Start,
      DK3D::XInputGamepadState::Button::Back,
      DK3D::XInputGamepadState::Button::DPadUp,
      DK3D::XInputGamepadState::Button::DPadDown,
      DK3D::XInputGamepadState::Button::DPadLeft,
      DK3D::XInputGamepadState::Button::DPadRight,
      DK3D::XInputGamepadState::Button::LS,
      DK3D::XInputGamepadState::Button::RS,
      DK3D::XInputGamepadState::Button::LB,
      DK3D::XInputGamepadState::Button::RB,
   };
   static_assert(DK3D::XInputGamepadState::button_count == _indices_to_buttons.size());

   size_t _button_to_index(DK3D::XInputGamepadState::Button b) {
      for (size_t i = 0; i < _indices_to_buttons.size(); ++i)
         if (_indices_to_buttons[i] == b)
            return i;
      assert(false);
      cobb::unreachable();
   }
}

namespace DK3D {
   void XInputGamepadState::ignoreAllDown() {
      auto& btn = this->buttons;
      for (size_t i = 0; i < button_count; ++i)
         if (btn.start[i] != zero_timestamp)
            btn.ignore.set(i, true);
   }
   void XInputGamepadState::update(timestamp_t now, bool connected, const DKXInputSubsystem::Gamepad& gs) {
      this->is_connected = connected;
      if (!connected) {
         auto& scl = this->scalars;
         auto& vec = this->vectors;
         scl.lt = scl.rt = 0;
         vec.ls = vec.rs = {};
         //
         auto& btn = this->buttons;
         btn.releases.clear();
         //
         return;
      }
      {
         auto& scl = this->scalars;
         scl.lt = gs.lt;
         scl.rt = gs.rt;
      }
      {
         auto& vec = this->vectors;
         vec.ls = gs.ls;
         vec.rs = gs.rs;
      }
      //
      auto& btn = this->buttons;
      for (size_t i = 0; i < button_count; ++i) {
         auto& start = btn.start[i];
         bool  held  = gs.isButtonDown(_indices_to_buttons[i]);
         if (start == zero_timestamp) {
            //
            // Key was up, last we checked.
            //
            btn.releases[i] = KeyReleaseType::None;
            if (held) {
               //
               // Key has been pressed.
               //
               start = now;
            }
         } else {
            //
            // Key was down, last we checked.
            //
            if (!held) {
               //
               // Key has been released.
               //
               btn.ignore.reset(i);
               float elapsed = elapsed_time(start, now);
               start = zero_timestamp;
               if (elapsed >= DK3D::defaults::boolean_input_hold_threshold) {
                  btn.releases[i] = KeyReleaseType::Hold;
               } else {
                  btn.releases[i] = KeyReleaseType::Tap;
               }
            }
         }
      }
   }

   KeyReleaseType XInputGamepadState::releaseType(Button btn) const {
      return this->buttons.releases[_button_to_index(btn)];
   }

   bool XInputGamepadState::isDown(Button btn, bool even_if_ignored) const {
      auto idx = _button_to_index(btn);
      bool down = (this->buttons.start[idx] != zero_timestamp);
      if (!down)
         return false;
      if (even_if_ignored)
         return down;
      return !this->buttons.ignore.test(idx);
   }
   timestamp_t XInputGamepadState::downWhen(Button btn) const {
      return this->buttons.start[_button_to_index(btn)];
   }

   KeyDownState XInputGamepadState::keyDownState(Button btn) const {
      if (!this->is_connected)
         return KeyDownState();
      //
      auto idx = _button_to_index(btn);
      //
      KeyDownState out;
      out.down_when = this->buttons.start[idx];
      out.is_down   = (out.down_when != zero_timestamp) && !this->buttons.ignore.test(idx);
      if (!out.is_down)
         out.released = this->buttons.releases[idx];
      return out;
   }
}