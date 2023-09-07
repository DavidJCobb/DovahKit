#pragma once
#include "helpers/keyboard/key.h"
#include "helpers/qt/keycodes.h"
#include "../enums/button_press_type.h"
#include "editor/subsystems/xinput/enums/button.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput::inputs {
   using xinput_button = subsystems::xinput::button;

   struct button {
      cobb::keyboard::key key;
      Qt::MouseButton     mouse   = Qt::MouseButton::NoButton;
      xinput_button       gamepad = xinput_button::none;

      constexpr bool operator==(const button& other) const {
         if (!this->key.is_same_as(other.key))
            return false;
         if (this->mouse != other.mouse)
            return false;
         if (this->gamepad != other.gamepad)
            return false;
         return true;
      }

      constexpr bool empty() const noexcept {
         return key.empty() && (mouse == Qt::MouseButton::NoButton) && (gamepad == xinput_button::none);
      }

      constexpr bool is_modifier_key() const noexcept {
         return key.is_modifier() && (mouse == Qt::MouseButton::NoButton) && (gamepad == xinput_button::none);
      }

      constexpr void stream(cobb::bitstreams::reader&);
      constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./button.inl"