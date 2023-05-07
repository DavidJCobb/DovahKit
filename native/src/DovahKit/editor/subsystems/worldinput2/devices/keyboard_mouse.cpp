#include "./keyboard_mouse.h"
#include "helpers/windows.h"
#include "../defaults.h"
#include "../interruption_check.h"
#include "../inputs/button.h"
#include <QCursor>

namespace {
   constexpr bool use_winapi_for_mouse_location = false;
}

namespace {
   bool vk_is_mouse(int vk) {
      switch (vk) {
         case VK_LBUTTON:
         case VK_RBUTTON:
         case VK_MBUTTON:
         case VK_XBUTTON1:
         case VK_XBUTTON2:
            return true;
      }
      return false;
   }
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
   int button_to_vk(const dovahkit::subsystems::worldinput2::inputs::button& b) {
      if (!b.key.empty()) {
         if (b.mouse != Qt::MouseButton::NoButton) {
            return qt_mouse_button_to_vk(b.mouse);
         }
         return (int)b.key.vk;
      }
      if (b.mouse != Qt::MouseButton::NoButton)
         return qt_mouse_button_to_vk(b.mouse);
      return -1;
   }
}

namespace dovahkit::subsystems::worldinput2::devices {
   bool keyboard_mouse::_mouseup_handler_for_double_click(timestamp_t now) {
      auto& last = this->mouse.last_click;
      if (last.time == timestamp_t{})
         return false;
      QPoint pos = QCursor::pos();
      pos -= this->mouse.pos;
      if (pos.x() > this->system.mouse.hitboxes.double_click.x())
         return false;
      if (pos.y() > this->system.mouse.hitboxes.double_click.y())
         return false;
      return true;
   }

   void keyboard_mouse::ignore_all_down() {
      this->buttons.ignore_all_down();
   }
   void keyboard_mouse::update(timestamp_t now) {
      for (size_t i = 0; i < vk_code_count; ++i) {
         bool held = (GetAsyncKeyState(i) & 0x8000) != 0;
         this->buttons.update_button(now, i, held);

         if (!held) {
            if (i == ((this->system.mouse.swap_left_right) ? VK_RBUTTON : VK_LBUTTON)) {
               if (_mouseup_handler_for_double_click(now)) {
                  //
                  // Double-click.
                  //
                  this->mouse.last_click.pos  = {}; // TODO: How does this fit into the original code? How does it fit into our new algorithms?
                  this->mouse.last_click.time = {};
                  //
                  // TODO: Find a way to signal double-clicks to DK3D for handling.
                  //
               } else {
                  //
                  // Not a double-click.
                  //
                  this->mouse.last_click.pos  = QCursor::pos();
                  this->mouse.last_click.time = now;
               }
            }
         }
      }
      if (auto& list = this->raycast_results; !list.empty()) {
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
                  auto vk = button_to_vk(result.button);
                  if (vk < 0)
                     return true;

                  constexpr auto down_or_released_this_frame = device_button_state::flag::is_down | device_button_state::flag::down_state_changed_on_this_frame;
                  //
                  auto f = this->buttons.flags[vk];
                  if ((f & down_or_released_this_frame) == 0) {
                     return true;
                  }

                  return false;
               }
            ),
            list.end()
         );
      }
      //
      // Get mousemove state:
      //
      if constexpr (use_winapi_for_mouse_location) {
         POINT current;
         if (GetCursorPos(&current) == 0) {
            #if _DEBUG
               auto err = GetLastError();
               __debugbreak();
            #endif
         } else {
            auto prior = this->mouse.pos;
            this->mouse.pos  = { current.x, current.y };
            this->mouse.move = this->mouse.pos - prior;
         }
      } else {
         QPoint current = QCursor::pos();
         this->mouse.move = current - this->mouse.pos;
         this->mouse.pos  = current;
      }
      if (this->mouse.move.isNull()) {
         auto elapsed = elapsed_time(this->mouse.last_movement, now);
         this->mouse.is_stale = (elapsed < 0.066);
      } else {
         this->mouse.is_stale = false;
         this->mouse.last_movement = now;
      }
      //
      // TODO: If we want mouse wheel state, we'll need to either find a way to bridge QWheelEvent to 
      // this system, or use Windows's "Raw Input" API.
      // 
      // https://docs.microsoft.com/en-us/windows/win32/inputdev/about-raw-input
      // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
      //
   }

   void keyboard_mouse::recheck_mouse_metrics() {
      auto& mouse = this->system.mouse;
      mouse.hitboxes.double_click.setX(GetSystemMetrics(SM_CXDOUBLECLK));
      mouse.hitboxes.double_click.setY(GetSystemMetrics(SM_CYDOUBLECLK));
      mouse.hitboxes.drag.setX(GetSystemMetrics(SM_CXDRAG));
      mouse.hitboxes.drag.setY(GetSystemMetrics(SM_CYDRAG));
      mouse.has_scroll_wheel = GetSystemMetrics(SM_MOUSEWHEELPRESENT) != 0;
      mouse.swap_left_right  = GetSystemMetrics(SM_SWAPBUTTON) != 0;
      mouse.double_click_ms  = GetDoubleClickTime();
   }

   device_button_state keyboard_mouse::get_state_of(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return {};
      return this->buttons.get_button_state(vk);
   }
   const device_button_claim& keyboard_mouse::get_existing_claim_of(const inputs::button& button) const {
      return this->buttons.claims[button_to_vk(button)].existing;
   }
   device_button_claim& keyboard_mouse::get_pending_claim_of(const inputs::button& button) {
      return this->buttons.claims[button_to_vk(button)].pending;
   }
   const device_button_claim& keyboard_mouse::get_pending_claim_of(const inputs::button& button) const {
      return this->buttons.claims[button_to_vk(button)].pending;
   }
   interruption_check keyboard_mouse::_prepare_interruption_check_impl() const {
      interruption_check check;
      for (size_t i = 0; i < vk_code_count; ++i) {
         if ((this->buttons.flags[i] & device_button_state::flag::is_down) == 0)
            continue;
         check.buttons.emplace_back(interruption_check::potentially_interrupting_button{
            inputs::button{
               .key = cobb::keyboard::key((cobb::keyboard::virtual_key)i),
            },
            this->buttons.down_at[i]
         });
      }
      return check;
   }
   //
   bool keyboard_mouse::button_is_valid(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return false;
      return true;
   }
   //
   range_control_state keyboard_mouse::get_range_control_state(scalar_input_control c, axis2D axis) const {
      switch (c) {
         case scalar_input_control::mouse_move:
            {
               auto n = (axis == axis2D::y) ? this->mouse.move.y() : this->mouse.move.x();
               if (!n) {
                  if (this->mouse.is_stale)
                     return range_control_state::stale;
                  return range_control_state::zeroed;
               }
            }
            return range_control_state::active;
      }
      return range_control_state::unavailable;
   }
   range_control_state keyboard_mouse::get_range_control_state(vector_input_control c) const {
      switch (c) {
         case vector_input_control::mouse_move:
            if (this->mouse.move.x() == 0 && this->mouse.move.y() == 0) {
               if (this->mouse.is_stale)
                  return range_control_state::stale;
               return range_control_state::zeroed;
            }
            return range_control_state::active;
      }
      return range_control_state::unavailable;
   }
   QPointF keyboard_mouse::get_range_control_value(scalar_input_control s, axis2D axis) const {
      if (s == scalar_input_control::none)
         return { 0, 0 };
      switch (s) {
         case scalar_input_control::mouse_move:
            if (axis == axis2D::y)
               return { (qreal)this->mouse.move.y(), 0 };
            return { (qreal)this->mouse.move.x(), 0 };
      }
      return { 0, 0 };
   }
   QPointF keyboard_mouse::get_range_control_value(vector_input_control v) const {
      if (v == vector_input_control::none)
         return { 0, 0 };
      switch (v) {
         case vector_input_control::mouse_move:
            return this->mouse.move;
      }
      return { 0, 0 };
   }
}