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
   int button_to_vk(const dovahkit::subsystems::worldinput::inputs::button& b) {
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

namespace dovahkit::subsystems::worldinput::devices {
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
      for (size_t i = 0; i < vk_code_count; ++i)
         if (this->buttons.start[i] != zero_timestamp)
            this->buttons.ignore.set(i, true);
   }
   void keyboard_mouse::update(timestamp_t now) {
      this->buttons.processed.reset();
      //
      for (size_t i = 0; i < vk_code_count; ++i) {
         auto& start = this->buttons.start[i];
         bool  held  = (GetAsyncKeyState(i) & 0x8000) != 0;
         if (start == zero_timestamp) {
            //
            // Key was up, last we checked.
            //
            this->buttons.releases[i] = button_release_type::none;
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
               if (i == (this->system.mouse.swap_left_right) ? VK_RBUTTON : VK_LBUTTON) {
                  if (_mouseup_handler_for_double_click(now)) {
                     //
                     // Double-click.
                     //
                     this->mouse.last_click.pos  = {};
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
               this->buttons.ignore.reset(i);
               float elapsed = elapsed_time(start, now);
               start = zero_timestamp;
               if (elapsed >= dovahkit::subsystems::worldinput::defaults::boolean_input_hold_threshold) {
                  this->buttons.releases[i] = button_release_type::hold;
               } else {
                  this->buttons.releases[i] = button_release_type::tap;
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

   void keyboard_mouse::mark_button_processed(const inputs::button& button) {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return;
      this->buttons.processed.set(vk);
   }
   bool keyboard_mouse::is_button_processed(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return false;
      return this->buttons.processed.test(vk);
   }

   button_release_type keyboard_mouse::release_type(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return button_release_type::none;
      return this->buttons.releases[vk];
   }
   bool keyboard_mouse::is_down(const inputs::button& button, bool even_if_ignored) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return false;
      bool down = (this->buttons.start[vk] != zero_timestamp);
      if (!down)
         return false;
      if (even_if_ignored)
         return down;
      return !this->buttons.ignore.test(vk);
   }
   timestamp_t keyboard_mouse::down_when(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      if (vk < 0 || vk >= vk_code_count)
         return zero_timestamp;
      return this->buttons.start[vk];
   }

   button_state keyboard_mouse::key_down_state(const inputs::button& button) const {
      auto vk = button_to_vk(button);
      //
      button_state out;
      out.down_when = this->buttons.start[vk];
      out.is_down   = (out.down_when != zero_timestamp) && !this->buttons.ignore.test(vk);
      if (!out.is_down)
         out.released = this->buttons.releases[vk];
      return out;
   }
}