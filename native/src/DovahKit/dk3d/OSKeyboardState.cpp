#include "OSKeyboardState.h"
#include "InputResult.h"
#include "defaults.h"
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

   constexpr MOUSEMOVEPOINT dummy_mouse_point = MOUSEMOVEPOINT{
      .x = 0,
      .y = 0,
      .time = 0,
      .dwExtraInfo = 0,
   };
}

namespace DK3D {
   void OSKeyboardState::ignoreAllDown() {
      for (size_t i = 0; i < vk_code_count; ++i)
         if (this->start[i] != zero_timestamp)
            this->ignore.set(i, true);
   }
   void OSKeyboardState::update(timestamp_t now) {
      for (size_t i = 0; i < vk_code_count; ++i) {
         auto& start = this->start[i];
         bool  held  = (GetAsyncKeyState(i) & 0x8000) != 0;
         if (start == zero_timestamp) {
            //
            // Key was up, last we checked.
            //
            this->releases[i] = KeyReleaseType::None;
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
               this->ignore.reset(i);
               float elapsed = elapsed_time(start, now);
               start = zero_timestamp;
               if (elapsed >= DK3D::defaults::boolean_input_hold_threshold) {
                  this->releases[i] = KeyReleaseType::Hold;
               } else {
                  this->releases[i] = KeyReleaseType::Tap;
               }
            }
         }
      }
      //
      // Get mousemove state:
      //
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
      //
      // TODO: If we want mouse wheel state, we'll need to either find a way to bridge QWheelEvent to 
      // this system, or use Windows's "Raw Input" API.
      // 
      // https://docs.microsoft.com/en-us/windows/win32/inputdev/about-raw-input
      // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
      //
   }

   void OSKeyboardState::recheckMouseMetrics() {
      this->system.mouse.hitboxes.double_click.setX(GetSystemMetrics(SM_CXDOUBLECLK));
      this->system.mouse.hitboxes.double_click.setY(GetSystemMetrics(SM_CYDOUBLECLK));
      this->system.mouse.hitboxes.drag.setX(GetSystemMetrics(SM_CXDRAG));
      this->system.mouse.hitboxes.drag.setY(GetSystemMetrics(SM_CYDRAG));
      this->system.mouse.has_scroll_wheel = GetSystemMetrics(SM_MOUSEWHEELPRESENT) != 0;
      this->system.mouse.swap_left_right  = GetSystemMetrics(SM_SWAPBUTTON) != 0;
   }

   KeyReleaseType OSKeyboardState::releaseType(int vk) const {
      if (vk < 0 || vk >= vk_code_count)
         return KeyReleaseType::None;
      return this->releases[vk];
   }
   KeyReleaseType OSKeyboardState::releaseType(Qt::MouseButton button) const {
      return this->releaseType(qt_mouse_button_to_vk(button));
   }

   bool OSKeyboardState::isDown(int vk, bool even_if_ignored) const {
      if (vk < 0 || vk >= vk_code_count)
         return false;
      bool down = (this->start[vk] != zero_timestamp);
      if (!down)
         return false;
      if (even_if_ignored)
         return down;
      return !this->ignore.test(vk);
   }
   bool OSKeyboardState::isDown(Qt::MouseButton button, bool even_if_ignored) const {
      return this->isDown(qt_mouse_button_to_vk(button), even_if_ignored);
   }

   timestamp_t OSKeyboardState::downWhen(int vk) const {
      return this->start[vk];
   }
   timestamp_t OSKeyboardState::downWhen(Qt::MouseButton button) const {
      return this->downWhen(qt_mouse_button_to_vk(button));
   }

   KeyDownState OSKeyboardState::keyDownState(int vk) const {
      KeyDownState out;
      out.down_when = this->start[vk];
      out.is_down   = (out.down_when != zero_timestamp) && !this->ignore.test(vk);
      if (!out.is_down)
         out.released = this->releases[vk];
      return out;
   }
   KeyDownState OSKeyboardState::keyDownState(Qt::MouseButton button) const {
      return this->keyDownState(qt_mouse_button_to_vk(button));
   }
}