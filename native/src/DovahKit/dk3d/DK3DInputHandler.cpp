#include "DK3DInputHandler.h"
#include "../editor/subsystems/DKXInputSubsystem.h"

using namespace DK3D;

namespace {
   constexpr float reset_after_lag_threshold = 3.0; // ignore all held inputs if this much time passed since we last polled

   constexpr double boolean_input_hold_threshold = 0.7;

   template<typename F, typename... T> inline void _for_each(F func, T&... args) {
      (func)(args...);
   };
}

DK3DInputHandler::DK3DInputHandler() {
   //
   // Hardcoded bindings for testing:
   //
   {
      auto& cb = this->binds.keyboard.camera.move;
      cb.forward = BoundInput::from_key('W');
      cb.back    = BoundInput::from_key('S');
      cb.left    = BoundInput::from_key('A');
      cb.right   = BoundInput::from_key('D');
      cb.up      = BoundInput::from_key('Q');
      cb.down    = BoundInput::from_key('Z');
   }
   {
      auto& ct = this->binds.keyboard.camera.turn;
      ct.left  = BoundInput::from_key('G');
      ct.right = BoundInput::from_key('H');
      ct.up    = BoundInput::from_key('R');
      ct.down  = BoundInput::from_key('V');
   }
   {
      auto& cb = this->binds.gamepad.camera.move;
      cb.lateral.input.vector.input = VectorControl::XInput_LS;
      cb.down.input.boolean.gamepad.button = XInputKey::LB;
      cb.up.input.boolean.gamepad.button = XInputKey::RB;
   }
   {
      auto& cb = this->binds.gamepad.camera.turn;
      cb.yaw.input.scalar.input = ScalarControl::XInput_RS;
      cb.yaw.input.scalar.axis  = Axis2D::X;
      cb.pitch.input.scalar.input = ScalarControl::XInput_RS;
      cb.pitch.input.scalar.axis  = Axis2D::Y;
   }
}
DK3DInputHandler::~DK3DInputHandler() {
}

float DK3DInputHandler::scalarControlValue(ScalarControl c, Axis2D axis) const {
   switch (c) {
      using _ = ScalarControl;
      case _::None:
         return 0;
      case _::MouseMove:
         if (axis == Axis2D::X)
            return this->state.mousemove.x;
         return this->state.mousemove.y;
      case _::MouseWheel:
         return this->state.mousemove.wheel;
   }
   auto& xi = DKXInputSubsystem::get();
   if (!xi.isGamepadConnected())
      return 0;
   auto& gs = xi.gamepadState();
   switch (c) {
      using _ = ScalarControl;
      case _::XInput_LT:
         return gs.lt;
      case _::XInput_RT:
         return gs.rt;
      case _::XInput_LS:
         if (axis == Axis2D::X)
            return gs.ls.x();
         return gs.ls.y();
      case _::XInput_RS:
         if (axis == Axis2D::X)
            return gs.rs.x();
         return gs.rs.y();
   }
   return 0;
}
QPointF DK3DInputHandler::vectorControlValue(VectorControl c) const {
   switch (c) {
      using _ = VectorControl;
      case _::None:
         return { 0, 0 };
      case _::MouseMove:
         return { this->state.mousemove.x, this->state.mousemove.y };
   }
   auto& xi = DKXInputSubsystem::get();
   if (!xi.isGamepadConnected())
      return { 0, 0 };
   auto& gs = xi.gamepadState();
   switch (c) {
      using _ = VectorControl;
      case _::XInput_LS:
         return gs.ls;
      case _::XInput_RS:
         return gs.rs;
   }
   return { 0, 0 };
}

void DK3DInputHandler::ignoreAllHeldKeys(timestamp_t now) {
   if (now == zero_timestamp)
      now = current_time();
   //
   for (size_t i = 0; i + 15 < vk_code_count; ++i) {
      if (this->keyboard_state.start[i] != zero_timestamp)
         this->keyboard_state.ignore.set(i, true);
   }
   //
   auto& km = this->binds.keyboard.camera.move;
   _for_each(
      [now](BoundInputState& s) { s.ignore_if_down(now); },
      km.forward,
      km.back,
      km.left,
      km.right,
      km.up,
      km.down
   );
   auto& kt = this->binds.keyboard.camera.turn;
   _for_each(
      [now](BoundInputState& s) { s.ignore_if_down(now); },
      kt.left,
      kt.right,
      kt.up,
      kt.down
   );
   auto& gp = this->binds.gamepad;
   _for_each(
      [now](BoundInputState& s) { s.ignore_if_down(now); },
      gp.test_tap,
      gp.test_hold,
      gp.test_while
   );
}
void DK3DInputHandler::updateAllKeys(timestamp_t now) {
   for (size_t i = 0; i + 15 < vk_code_count; ++i) {
      auto& start = this->keyboard_state.start[i];
      bool  held  = (GetAsyncKeyState(i) & 0x8000) != 0;
      if (start == zero_timestamp) {
         //
         // Key was up, last we checked.
         //
         this->keyboard_state.releases[i] = key_release_type::none;
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
            this->keyboard_state.ignore.reset(i);
            float elapsed = elapsed_time(start, now);
            start = zero_timestamp;
            if (elapsed >= boolean_input_hold_threshold) {
               this->keyboard_state.releases[i] = key_release_type::hold;
            } else {
               this->keyboard_state.releases[i] = key_release_type::tap;
            }
         }
      }
   }
   //
   // TODO: We'd want to make it so that instead of calling a function on 
   // BoundInputState to update it, we just have a function on this singleton 
   // that takes a (const BoundInput&) and returns a struct which indicates...
   // 
   //  - X/Y for scalar and vector inputs
   //  - Boolean indicating whether the boolean input is currently "active"
   //     - Given {const auto& ks = this->keyboard_state}:
   //        - An input is "down" if (ks.start[i] != zero_timestamp)
   //        - "While" inputs are active while the input is down
   //        - "Hold" inputs are active if ks.releases[i] is key_release_type::hold
   //        - "Tap" inputs are active if ks.releases[i] is key_release_type::tap
   // 
   // Then, we'd want to have our "update" slot pass the BoundInputStates' 
   // BoundInput members to that function. (Right now, we could replace the 
   // BoundInputState struct with just BoundInput. In the future, I'd like 
   // to have a more comprehensive struct -- the tree hierarchy I've planned, 
   // with each struct indicating the target editor function and arguments -- 
   // but for now, let's do one thing at a time.)
   //
   auto& km = this->binds.keyboard.camera.move;
   _for_each(
      [now](BoundInputState& s) { s.update(now); },
      km.forward,
      km.back,
      km.left,
      km.right,
      km.up,
      km.down
   );
   auto& kt = this->binds.keyboard.camera.turn;
   _for_each(
      [now](BoundInputState& s) { s.update(now); },
      kt.left,
      kt.right,
      kt.up,
      kt.down
   );
   auto& gp = this->binds.gamepad;
   _for_each(
      [now](BoundInputState& s) { s.update(now); },
      gp.test_tap,
      gp.test_hold,
      gp.test_while
   );
}

DKVulkanCameraUpdate DK3DInputHandler::update(DKVulkanView* subject) {
   if (subject != this->state.target_view)
      return DKVulkanCameraUpdate();
   //
   auto now     = DK3D::current_time();
   auto elapsed = DK3D::elapsed_time(this->state.last_update, now);
   if (elapsed <= 0.0)
      //
      // This can happen sometimes -- we receive  an update so soon that it's not even 
      // an easily-measurable  fraction of a  second -- and in  that case, there's not 
      // any point to doing input processing. We'll just be scaling speeds and whatnot 
      // by zero anyway.
      //
      return DKVulkanCameraUpdate();
   this->state.last_update = now;
   //
   if (elapsed >= reset_after_lag_threshold) {
      this->ignoreAllHeldKeys(now);
      return DKVulkanCameraUpdate();
   }
   //
   auto& xinput = DKXInputSubsystem::get();
   xinput.update();
   bool has_gamepad = xinput.isGamepadConnected();
   //
   this->updateAllKeys(now);
   //
   if (has_gamepad) {
      auto& gp = this->binds.gamepad;
      if (gp.test_tap.last_result.active()) {
         qDebug("[DK3DInputHandler] \"Tap\" test bind activated.");
      }
      if (gp.test_hold.last_result.active()) {
         qDebug("[DK3DInputHandler] \"Hold\" test bind activated.");
      }
      auto tw = gp.test_while.last_result;
      if (tw.active() && tw.while_has_changed) {
         qDebug("[DK3DInputHandler] \"While\" test bind has started...");
      } else if (!tw.active() && tw.while_has_changed) {
         qDebug("[DK3DInputHandler] \"While\" test bind has ended.");
      }
   }
   DKVulkanCameraUpdate update;
   update.delta_seconds = elapsed;
   update.move.speed    = 1.0;
   {
      auto& km = this->binds.keyboard.camera.move;
      auto& cm = update.move.direction;
      if (km.forward.last_result.active()) {
         cm.y += 1;
      }
      if (km.back.last_result.active()) {
         cm.y -= 1;
      }
      if (km.left.last_result.active()) {
         cm.x -= 1;
      }
      if (km.right.last_result.active()) {
         cm.x += 1;
      }
      if (km.up.last_result.active()) {
         cm.z += 1;
      }
      if (km.down.last_result.active()) {
         cm.z -= 1;
      }
      //
      if (has_gamepad) {
         auto& gm = this->binds.gamepad.camera.move;
         if (gm.lateral.last_result.active()) {
            auto& lr = gm.lateral.last_result;
            cm.x += lr.x; // stick right is positive; left is negative
            cm.y += lr.y; // stick up    is positive; down is negative
         }
         if (gm.up.last_result.active()) {
            cm.z += 1;
         }
         if (gm.down.last_result.active()) {
            cm.z -= 1;
         }
      }
   }
   update.turn.speed = glm::radians(90.0F);
   {
      auto& km = this->binds.keyboard.camera.turn;
      auto& cm = update.turn;
      if (km.left.last_result.active()) {
         cm.yaw += 1;
      }
      if (km.right.last_result.active()) {
         cm.yaw -= 1;
      }
      if (km.up.last_result.active()) {
         cm.pitch -= 1;
      }
      if (km.down.last_result.active()) {
         cm.pitch += 1;
      }
      if (has_gamepad) {
         auto& gm = this->binds.gamepad.camera.turn;
         if (gm.yaw.last_result.active()) {
            auto& lr = gm.yaw.last_result;
            cm.yaw += lr.x;
         }
         if (gm.pitch.last_result.active()) {
            auto& lr = gm.yaw.last_result;
            cm.pitch += lr.x;
         }
      }
      double speed = sqrt((cm.yaw * cm.yaw) + (cm.pitch * cm.pitch));
      speed = std::clamp(speed, 0.0, 1.0);
      update.turn.speed *= speed;
   }
   return update;
}