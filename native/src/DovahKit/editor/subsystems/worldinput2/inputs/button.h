#pragma once
#include "helpers/qt/keycodes.h"
#include "../enums/button_press_type.h"
#include "editor/subsystems/xinput/enums/button.h"

namespace dovahkit::subsystems::worldinput2::inputs {
   using xinput_button = subsystems::xinput::button;

   struct button {
      cobb::qt::key   key;
      Qt::MouseButton mouse   = Qt::MouseButton::NoButton;
      xinput_button   gamepad = xinput_button::none;

      inline bool empty() const noexcept {
         return key.empty() && (mouse == Qt::MouseButton::NoButton) && (gamepad == xinput_button::none);
      }

      bool is_modifier_key() const noexcept {
         return key.is_modifier_key() && (mouse == Qt::MouseButton::NoButton) && (gamepad == xinput_button::none);
      }
   };
}