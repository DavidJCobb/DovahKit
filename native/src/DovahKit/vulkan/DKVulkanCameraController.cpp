#include "DKVulkanCameraController.h"
#include "../editor/DovahKit3DControls.h"
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "../helpers/qt/keycodes.h"

#include <windows.h>
#include "../helpers/intrusive_windows_defines.h"

namespace {
   constexpr float reset_after_lag_threshold = 3.0; // ignore all held inputs if this much time passed since we last polle
}

DKVulkanCameraController::DKVulkanCameraController(QObject* parent) : QObject(parent) {
   this->updateBindings();
   QObject::connect(&DovahKit3DControls::get(), &DovahKit3DControls::bindingsChanged, this, &DKVulkanCameraController::updateBindings);
}

bool DKVulkanCameraController::_Key::check(timestamp_t now) {
   bool down = GetAsyncKeyState(this->vk) & 0x8000;
   if (!down) {
      this->down_at = not_down;
      this->ignore  = false;
      return false;
   }
   if (this->down_at == not_down) {
      this->down_at = now;
   } else {
      return !this->ignore;
   }
   return true;
}
void DKVulkanCameraController::_Key::ignore_if_down() {
   if (this->is_down())
      this->ignore = true;
}
void DKVulkanCameraController::_Key::set(const cobb::qt::key& bind) {
   this->down_at = not_down;
   this->vk      = bind.native.vk;
}

DKVulkanCameraUpdate DKVulkanCameraController::poll() {
   constexpr float epsilon = 0.00001;
   using elapsed_t = std::chrono::duration<double, std::chrono::seconds::period>;

   auto now     = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
   auto elapsed = (this->state.last_poll == not_down) ? (1.0 / 60) : elapsed_t(now - this->state.last_poll).count();
   if (elapsed <= 0.0)
      //
      // This can happen sometimes -- we receive  an update so soon that it's not even 
      // an easily-measurable  fraction of a  second -- and in  that case, there's not 
      // any point to doing input processing. We'll just be scaling speeds and whatnot 
      // by zero anyway.
      //
      return DKVulkanCameraUpdate();
   this->state.last_poll = now;
   //
   if (elapsed >= reset_after_lag_threshold) {
      auto& km = this->state.keyboard.camera.move;
      km.forward.ignore_if_down();
      km.back.ignore_if_down();
      km.left.ignore_if_down();
      km.right.ignore_if_down();
      km.up.ignore_if_down();
      km.down.ignore_if_down();
      auto& kt = this->state.keyboard.camera.turn;
      kt.left.ignore_if_down();
      kt.right.ignore_if_down();
      kt.up.ignore_if_down();
      kt.down.ignore_if_down();
      return DKVulkanCameraUpdate();
   }
   //
   // Knowing the camera axes will be important for handling this stuff.
   // 
   //    X = Side   = positive is left
   //    Y = Height = positive is down (since OpenGL is inverted)
   //    Z = Depth  = positive is forward
   // 
   // For rotations, that means:
   // 
   //    X = Pitch (tilt nose about the side axis)
   //    Y = Yaw   (twist heading about the vertical axis)
   //    Z = Lean  (tilt nose about the forward axis)
   //
   auto& xinput = DKXInputSubsystem::get();
   xinput.update();
   bool  has_gamepad   = xinput.isGamepadConnected();
   auto& gamepad_state = xinput.gamepadState();
   //
   DKVulkanCameraUpdate update;
   update.delta_seconds = elapsed;
   update.move.speed    = 1.0;
   {
      auto& km = this->state.keyboard.camera.move;
      auto& cm = update.move.direction;
      if (km.forward.check(now)) {
         cm.y += 1;
      }
      if (km.back.check(now)) {
         cm.y -= 1;
      }
      if (km.left.check(now)) {
         cm.x -= 1;
      }
      if (km.right.check(now)) {
         cm.x += 1;
      }
      if (km.up.check(now)) {
         cm.z += 1;
      }
      if (km.down.check(now)) {
         cm.z -= 1;
      }
      if (has_gamepad) {
         const auto& stick = gamepad_state.ls;
         cm.x += stick.x();
         cm.y -= stick.y(); // stick up is negative; object forward is positive; need a sign flip
         //
         if (gamepad_state.isButtonDown(DKXInputSubsystem::Button::LB)) { // LB = down
            cm.z -= 1;
         }
         if (gamepad_state.isButtonDown(DKXInputSubsystem::Button::RB)) { // RB = up
            cm.z += 1;
         }
      }
   }
   update.turn.speed = glm::radians(90.0F);
   {
      auto& km = this->state.keyboard.camera.turn;
      auto& cm = update.turn;
      if (km.left.check(now)) {
         cm.yaw += 1;
      }
      if (km.right.check(now)) {
         cm.yaw -= 1;
      }
      if (km.up.check(now)) {
         cm.pitch -= 1;
      }
      if (km.down.check(now)) {
         cm.pitch += 1;
      }
      if (has_gamepad) {
         const auto& stick = gamepad_state.rs;
         cm.yaw   += stick.y();
         cm.pitch -= stick.x();
         //
         auto speed = sqrt((stick.x() * stick.x()) + (stick.y() * stick.y()));
         speed = std::clamp(speed, 0.0, 1.0);
         update.turn.speed *= speed;
      }
   }
   return update;
}

void DKVulkanCameraController::updateBindings() {
   auto& controls = DovahKit3DControls::get();
   {
      auto& dst = this->state.keyboard.camera.move;
      auto& src = controls.keyboard.camera.move;
      dst.forward.set(src.forward);
      dst.back.set(src.back);
      dst.left.set(src.left);
      dst.right.set(src.right);
      dst.up.set(src.up);
      dst.down.set(src.down);
   }
   {
      auto& dst = this->state.keyboard.camera.turn;
      auto& src = controls.keyboard.camera.turn;
      dst.left.set(src.left);
      dst.right.set(src.right);
      dst.up.set(src.up);
      dst.down.set(src.down);
   }
}