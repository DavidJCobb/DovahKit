#include "xinput.h"
#include "helpers/unreachable.h"
#include "dk3d/defaults.h"
#include "dk3d/inputs/button.h"

namespace {
   using Button = DK3D::devices::xinput::Button;

   constexpr std::array _indices_to_buttons = {
      Button::A,
      Button::B,
      Button::X,
      Button::Y,
      Button::Start,
      Button::Back,
      Button::DPadUp,
      Button::DPadDown,
      Button::DPadLeft,
      Button::DPadRight,
      Button::LS,
      Button::RS,
      Button::LB,
      Button::RB,
      Button::LT,
      Button::RT,
   };
   static_assert(DK3D::devices::xinput::button_count == _indices_to_buttons.size());

   constexpr size_t no_button = -1;
   //
   size_t _button_to_index(const DK3D::inputs::button& button) {
      auto b = button.gamepad;
      for (size_t i = 0; i < _indices_to_buttons.size(); ++i)
         if (_indices_to_buttons[i] == b)
            return i;
      return no_button;
   }
}

namespace DK3D::devices {
   void xinput::ignore_all_down() {
      auto& btn = this->buttons;
      for (size_t i = 0; i < button_count; ++i)
         if (btn.start[i] != zero_timestamp)
            btn.ignore.set(i, true);
   }
   void xinput::update(timestamp_t now, bool connected, const DKXInputSubsystem::Gamepad& gs) {
      this->is_connected = connected;
      //
      this->buttons.processed.reset();
      //
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

   void xinput::mark_button_processed(const inputs::button& b) {
      auto i = _button_to_index(b);
      if (i == no_button)
         return;
      this->buttons.processed.set(i);
   }
   bool xinput::is_button_processed(const inputs::button& b) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return false;
      return this->buttons.processed.test(i);
   }

   KeyReleaseType xinput::release_type(const inputs::button& b) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return KeyReleaseType::None;
      return this->buttons.releases[i];
   }

   bool xinput::is_down(const inputs::button& b, bool even_if_ignored) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return false;
      bool down = (this->buttons.start[i] != zero_timestamp);
      if (!down)
         return false;
      if (even_if_ignored)
         return down;
      return !this->buttons.ignore.test(i);
   }
   timestamp_t xinput::down_when(const inputs::button& b) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return zero_timestamp;
      return this->buttons.start[i];
   }

   KeyDownState xinput::key_down_state(const inputs::button& b) const {
      if (!this->is_connected)
         return KeyDownState();
      auto i = _button_to_index(b);
      if (i == no_button)
         return KeyDownState();
      //
      KeyDownState out;
      out.down_when = this->buttons.start[i];
      out.is_down   = (out.down_when != zero_timestamp) && !this->buttons.ignore.test(i);
      if (!out.is_down)
         out.released = this->buttons.releases[i];
      return out;
   }
}