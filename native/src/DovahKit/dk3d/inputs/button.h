#pragma once
#include "helpers/qt/keycodes.h"
#include "dk3d/enums/button_press_type.h"
#include "editor/subsystems/DKXInputSubsystem.h"

namespace DK3D::inputs {
   using xinput_button = DKXInputSubsystem::Button;

   struct button {
      cobb::qt::key   key;
      Qt::MouseButton mouse   = Qt::MouseButton::NoButton;
      xinput_button   gamepad = xinput_button::None;
      //
      button_press_type press_type = button_press_type::tap;

      inline bool empty() const noexcept {
         return key.empty() && (mouse == Qt::MouseButton::NoButton) && (gamepad == xinput_button::None);
      }
   };
}