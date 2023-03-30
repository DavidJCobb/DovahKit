#include "keyboard_mouse.h"
#include "helpers/windows.h"
#include "../defaults.h"
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
         return b.key.native.vk;
      }
      if (b.mouse != Qt::MouseButton::NoButton)
         return qt_mouse_button_to_vk(b.mouse);
      return -1;
   }

   constexpr MOUSEMOVEPOINT dummy_mouse_point = MOUSEMOVEPOINT{
      .x = 0,
      .y = 0,
      .time = 0,
      .dwExtraInfo = 0,
   };
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
            if (i == (this->system.mouse.swap_left_right) ? VK_RBUTTON : VK_LBUTTON) {
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
      //
      // Get mousemove state:
      //
      if constexpr (use_winapi_for_mouse_location) {
         auto dummy = dummy_mouse_point;
         std::array<MOUSEMOVEPOINT, 1> points = {};
         auto count = GetMouseMovePointsEx(sizeof(MOUSEMOVEPOINT), &dummy, points.data(), 1, GMMP_USE_DISPLAY_POINTS);
         if (count > 0) {
            auto& point = points[count - 1];
            //
            // Multiple monitors may require some normalization:
            //
            if (point.x > 32767)
               point.x -= 65536;
            if (point.y > 32767)
               point.y -= 65536;
            //
            auto prior = this->mouse.pos;
            this->mouse.pos  = { point.x, point.y };
            this->mouse.move = this->mouse.pos - prior;
         }
      } else {
         QPoint current = QCursor::pos();
         this->mouse.move = current - this->mouse.pos;
         this->mouse.pos  = current;
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
   void keyboard_mouse::consume(const inputs::button& button) {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return;
      this->buttons.flags[vk] |= device_button_state::flag::consumed_on_this_frame;
   }

   button_press_type keyboard_mouse::release_type(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return button_press_type::none;
      return this->buttons.release_times[vk];
   }
   bool keyboard_mouse::is_down(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return false;
      return (this->buttons.start[vk] != zero_timestamp);
   }
   timestamp_t keyboard_mouse::down_when(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return zero_timestamp;
      return this->buttons.start[vk];
   }
}