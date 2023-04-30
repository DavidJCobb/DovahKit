#include "./xinput.h"
#include <array>
#include "helpers/unreachable.h"
#include "../inputs/button.h"
#include "../defaults.h"
#include "../interruption_check.h"

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
   const device_button_claim& xinput::get_existing_claim_of(const inputs::button& button) const {
      return this->buttons.claims[_button_to_index(button)].existing;
   }
   device_button_claim& xinput::get_pending_claim_of(const inputs::button& button) {
      return this->buttons.claims[_button_to_index(button)].pending;
   }
   const device_button_claim& xinput::get_pending_claim_of(const inputs::button& button) const {
      return this->buttons.claims[_button_to_index(button)].pending;
   }
   interruption_check xinput::_prepare_interruption_check_impl() const {
      interruption_check check;
      for (size_t i = 0; i < button_count; ++i) {
         if ((this->buttons.flags[i] & device_button_state::flag::is_down) == 0)
            continue;
         check.buttons.emplace_back(interruption_check::potentially_interrupting_button{
            inputs::button{
               .gamepad = _indices_to_buttons[i],
            },
            this->buttons.down_at[i]
         });
      }
      return check;
   }
   //
   bool xinput::button_is_valid(const inputs::button& button) const {
      auto i = _button_to_index(button);
      return (i != no_button);
   }
   //
   range_control_state xinput::get_range_control_state(scalar_input_control c, axis2D axis) const {
      qreal scalar;
      switch (c) {
         case scalar_input_control::xinput_ls:
            scalar = (axis == axis2D::y) ? this->vectors.ls.y() : this->vectors.ls.x();
            break;
         case scalar_input_control::xinput_rs:
            scalar = (axis == axis2D::y) ? this->vectors.rs.y() : this->vectors.rs.x();
            break;
         case scalar_input_control::xinput_lt:
            scalar = this->scalars.lt;
            break;
         case scalar_input_control::xinput_rt:
            scalar = this->scalars.rt;
            break;
         default:
            return range_control_state::unavailable;
      }
      return (scalar == 0.0) ? range_control_state::zeroed : range_control_state::active;
   }
   range_control_state xinput::get_range_control_state(vector_input_control c) const {
      switch (c) {
         case vector_input_control::xinput_ls:
            return this->vectors.ls.isNull() ? range_control_state::zeroed : range_control_state::active;
         case vector_input_control::xinput_rs:
            return this->vectors.rs.isNull() ? range_control_state::zeroed : range_control_state::active;
      }
      return range_control_state::unavailable;
   }
   QPointF xinput::get_range_control_value(scalar_input_control s, axis2D axis) const {
      if (s == scalar_input_control::none)
         return { 0, 0 };
      switch (s) {
         case scalar_input_control::xinput_ls:
            if (axis == axis2D::y)
               return { this->vectors.ls.y(), 0 };
            return { this->vectors.ls.x(), 0 };
         case scalar_input_control::xinput_rs:
            if (axis == axis2D::y)
               return { this->vectors.rs.y(), 0 };
            return { this->vectors.rs.x(), 0 };
         case scalar_input_control::xinput_lt:
            return { this->scalars.lt, 0 };
         case scalar_input_control::xinput_rt:
            return { this->scalars.rt, 0 };
      }
      return { 0, 0 };
   }
   QPointF xinput::get_range_control_value(vector_input_control v) const {
      if (v == vector_input_control::none)
         return { 0, 0 };
      switch (v) {
         case vector_input_control::xinput_ls:
            return this->vectors.ls;
         case vector_input_control::xinput_rs:
            return this->vectors.rs;
      }
      return { 0, 0 };
   }
}