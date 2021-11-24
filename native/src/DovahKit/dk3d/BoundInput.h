#pragma once
#include "../helpers/qt/keycodes.h"
#include "../editor/subsystems/DKXInputSubsystem.h"

namespace DK3D {
   enum class Axis2D {
      X,
      Y,
   };
   enum class BooleanInputMod {
      Tap,   // execute the function when the key is released, if it was held only briefly
      Hold,  // execute the function when the key is released, if it was held for a little while
      While, // execute the function continuously while the key is held, or (if it's a toggle) activate it on press and deactivate it on release
      DoubleClick, // mouse-only. same as Tap, but requires a double click
   };
   enum class ScalarControl { // A physical input that can produce a scalar value, e.g. a joystick trigger or a single axis on a joystick or mouse movement
      None,
      MouseMove,
      MouseWheel,
      XInput_LS,
      XInput_RS,
      XInput_LT,
      XInput_RT,
   };
   enum class VectorControl { // A physical input that can produce two scalar values, e.g. the movement of a joystick or the mouse
      None,
      MouseMove,
      XInput_LS,
      XInput_RS,
   };

   using XInputKey = DKXInputSubsystem::Button;

   struct BoundInput {
      struct {
         cobb::qt::key key;
         struct {
            Qt::MouseButton button = Qt::MouseButton::NoButton;
         } mouse;
         struct {
            XInputKey button = XInputKey::None;
         } gamepad;
         BooleanInputMod type = BooleanInputMod::Tap;
      } boolean;
      struct {
         ScalarControl input = ScalarControl::None;
         Axis2D axis = Axis2D::X;
      } scalar;
      struct {
         VectorControl input = VectorControl::None;
      } vector;

      inline bool is_boolean() const noexcept {
         auto& b = this->boolean;
         return (!b.key.empty()) || (b.gamepad.button != XInputKey::None) || (b.mouse.button != Qt::MouseButton::NoButton);
      }
      inline bool is_scalar() const noexcept {
         return !is_boolean() && this->scalar.input != ScalarControl::None;
      }
      inline bool is_vector() const noexcept {
         return !is_boolean() && !is_scalar() && this->vector.input != VectorControl::None;
      }

      inline bool is_mouse_boolean() const noexcept {
         auto& b = this->boolean;
         return b.key.empty() && (b.mouse.button != Qt::MouseButton::NoButton);
      }

      bool is_boolean_down() const; // checks whether the key is down. this cannot, on its own, process BooleanInputMods. it also can't enforce the "only catch mousedown on the 3D view" constraint.

      static BoundInput from_key(QChar, BooleanInputMod mod = BooleanInputMod::Tap);
      static BoundInput from_xinput_button(XInputKey, BooleanInputMod mod = BooleanInputMod::Tap);
   };
}