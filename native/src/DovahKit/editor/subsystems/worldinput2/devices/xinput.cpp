#include "xinput.h"
#include <array>
#include "helpers/unreachable.h"
#include "../defaults.h"
#include "../inputs/button.h"

namespace {
   using xinput_button = dovahkit::subsystems::xinput::button;

   constexpr std::array _indices_to_buttons = {
      xinput_button::a,
      xinput_button::b,
      xinput_button::x,
      xinput_button::y,
      xinput_button::start,
      xinput_button::back,
      xinput_button::d_pad_up,
      xinput_button::d_pad_down,
      xinput_button::d_pad_left,
      xinput_button::d_pad_right,
      xinput_button::stick_click_left,
      xinput_button::stick_click_right,
      xinput_button::bumper_left,
      xinput_button::bumper_right,
      xinput_button::trigger_left,
      xinput_button::trigger_right,
   };
   static_assert(dovahkit::subsystems::worldinput2::devices::xinput::button_count == _indices_to_buttons.size());

   constexpr size_t no_button = -1;
   //
   size_t _button_to_index(const dovahkit::subsystems::worldinput2::inputs::button& button) {
      auto b = button.gamepad;
      for (size_t i = 0; i < _indices_to_buttons.size(); ++i)
         if (_indices_to_buttons[i] == b)
            return i;
      return no_button;
   }
}

namespace dovahkit::subsystems::worldinput2::devices {
   void xinput::ignore_all_down() {
      this->buttons.ignore_all_down();
   }
   void xinput::update(timestamp_t now, bool connected, const subsystems::xinput::gamepad& gs) {
      auto was_connected = this->is_connected;
      this->is_connected = connected;
      //
      if (!connected) {
         auto& scl = this->scalars;
         auto& vec = this->vectors;
         scl.lt = scl.rt = 0;
         vec.ls = vec.rs = {};
         //
         if (was_connected)
            this->buttons.handle_disconnected();
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
         bool  held  = gs.is_button_down(_indices_to_buttons[i]);
         this->buttons.update_button(now, i, held);
      }
   }

   device_button_state xinput::get_state_of(const inputs::button& button) const {
      if (!this->is_connected)
         return {};
      auto i = _button_to_index(button);
      if (i == no_button)
         return {};
      return this->buttons.get_button_state(i);
   }
   bool xinput::is_consumed(const inputs::button& button) const {
      if (!this->is_connected)
         return true;
      auto i = _button_to_index(button);
      if (i == no_button)
         return {};
      return this->buttons.is_consumed(i);
   }
   void xinput::consume(const inputs::button& button) {
      auto i = _button_to_index(button);
      if (i == no_button)
         return;
      this->buttons.flags[i] |= device_button_state::flag::consumed_on_this_frame;
   }

   button_press_type xinput::release_type(const inputs::button& b) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return button_press_type::none;
      return this->buttons.release_times[i];
   }

   bool xinput::is_down(const inputs::button& b) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return false;
      return (this->buttons.start[i] != zero_timestamp);
   }
   timestamp_t xinput::down_when(const inputs::button& b) const {
      auto i = _button_to_index(b);
      if (i == no_button)
         return zero_timestamp;
      return this->buttons.start[i];
   }
}