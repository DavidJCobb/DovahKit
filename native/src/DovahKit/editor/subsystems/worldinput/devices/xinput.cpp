#include "./xinput.h"
#include <array>
#include "helpers/unreachable.h"
#include "../inputs/button.h"
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
   static_assert(dovahkit::subsystems::worldinput::devices::xinput::button_count == _indices_to_buttons.size());

   constexpr size_t no_button = -1;
   //
   size_t _button_to_index(const dovahkit::subsystems::worldinput::inputs::button& button) {
      auto b = button.gamepad;
      for (size_t i = 0; i < _indices_to_buttons.size(); ++i)
         if (_indices_to_buttons[i] == b)
            return i;
      return no_button;
   }
}

namespace dovahkit::subsystems::worldinput::devices {
   void xinput::ignore_all_down() {
      this->buttons.ignore_all_down();
   }

   void xinput::update(timestamp_t now, const QWidget& view, bool connected, const subsystems::xinput::gamepad& gs) {
      this->raycast_results.this_frame = {};
      this->update_buttons(now, connected, gs);
      this->update_pointer(view);
   }
   void xinput::update_buttons(timestamp_t now, bool connected, const subsystems::xinput::gamepad& gs) {
      auto was_connected = this->is_connected;
      this->is_connected = connected;
      //
      if (!connected) {
         auto& scl = this->scalars;
         auto& vec = this->vectors;
         scl.lt = scl.rt = 0;
         vec.ls = vec.rs = {};
         //
         if (was_connected) {
            this->buttons.handle_disconnected();
            this->raycast_results.per_button.clear();
         }
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
      if (auto& list = this->raycast_results.per_button; !list.empty()) {
         //
         // Clear raycast results for any buttons that were released on the previous frame. 
         // (We don't clear a button's results on the frame it's released, because that 
         // would prevent Press and Long Press binds from checking the results when they're 
         // about to activate.)
         //
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [this](const auto& result) -> bool {
                  auto i = _button_to_index(result.button);
                  if (i == no_button)
                     return true;

                  constexpr auto down_or_released_this_frame = device_button_state::flag::is_down | device_button_state::flag::down_state_changed_on_this_frame;
                  //
                  auto f = this->buttons.flags[i];
                  if ((f & down_or_released_this_frame) == 0) {
                     return true;
                  }

                  return false;
               }
            ),
            list.end()
         );
      }
   }
   void xinput::update_pointer(const QWidget& view) {
      auto r = view.rect();

      this->pointer_position.setX(r.left() + r.width() / 2);
      this->pointer_position.setY(r.top() + r.height() / 2); // centered reticle, for now
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
   range_control_state xinput::get_range_control_state(range_input_control c, range_input_axes axes) const {
      QPointF value;
      switch (c) {
         case range_input_control::xinput_ls: value = this->vectors.ls; break;
         case range_input_control::xinput_rs: value = this->vectors.rs; break;
         case range_input_control::xinput_lt: value = { this->scalars.lt, 0 }; break;
         case range_input_control::xinput_rt: value = { this->scalars.rt, 0 }; break;
         default:
            range_control_state::unavailable;
      }
      bool zeroed = true;
      if (range_input_control_has_multiple_axes(c)) {
         switch (axes) {
            using enum range_input_axes;
            case all:
               zeroed = value.isNull();
               break;
            case x: zeroed = value.x() == 0.0; break;
            case y: zeroed = value.y() == 0.0; break;
         }
      } else {
         zeroed = (value.x() == 0.0);
      }
      return zeroed ? range_control_state::zeroed : range_control_state::active;
   }
   QPointF xinput::get_range_control_value(range_input_control c, range_input_axes axes) const {
      if (c == range_input_control::none)
         return { 0, 0 };
      switch (c) {
         case range_input_control::xinput_ls:
         case range_input_control::xinput_rs:
            {
               const auto& src = (c == range_input_control::xinput_ls) ? this->vectors.ls : this->vectors.rs;
               switch (axes) {
                  using enum range_input_axes;
                  case x:   return { src.x(), 0 };
                  case y:   return { src.y(), 0 };
                  case all:
                  default:
                     return src;
               }
            }
            break;
         case range_input_control::xinput_lt:
            return { this->scalars.lt, 0 };
         case range_input_control::xinput_rt:
            return { this->scalars.rt, 0 };
      }
      return { 0, 0 };
   }
}