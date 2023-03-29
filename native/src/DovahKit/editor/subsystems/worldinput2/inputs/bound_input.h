#pragma once
#include <vector>
#include "../enums/button_press_type.h"
#include "./button.h"

namespace dovahkit::subsystems::worldinput2::inputs {
   struct bound_input {
      struct unordered_inputs {
         std::vector<button> buttons;
      };
      struct concurrent_inputs {
         std::vector<unordered_inputs> items;
      };

      button_press_type press_type = button_press_type::press;
      std::vector<concurrent_inputs> keys;

      constexpr bool empty() const {
         if (this->keys.empty())
            return true;
         for (const auto& group : this->keys) {
            if (group.items.empty())
               continue;
            for (const auto& unordered : group.items) {
               if (unordered.buttons.empty())
                  continue;
               for (const auto& b : unordered.buttons)
                  if (!b.empty())
                     return false;
            }
         }
         return true;
      }

      constexpr bool is_accelerator_key(bool require_modifiers = false) const {
         if (this->keys.size() != 1)
            return false;
         const auto& group = this->keys.back();
         if (group.items.size() == 2) {
            //
            // Modifier key(s), followed by one non-modifier key.
            //
            for (auto& btn : group.items[0].buttons) {
               if (!btn.is_modifier_key())
                  return false;
            }
            const auto& trigger = group.items[1];
            if (trigger.buttons.size() != 1)
               return false;
            if (trigger.buttons[0].key.is_modifier_key())
               return false;
            if (trigger.buttons[0].mouse != Qt::MouseButton::NoButton)
               return false;
            if (trigger.buttons[0].gamepad != xinput_button::none)
               return false;
            return true;
         }
         if (require_modifiers) {
            return false;
         }
         if (group.items.size() == 1) {
            const auto& trigger = group.items[0];
            if (trigger.buttons.size() != 1)
               return false;
            const auto& btn = trigger.buttons[0];
            if (btn.key.is_modifier_key())
               return false;
            if (trigger.buttons[0].mouse != Qt::MouseButton::NoButton)
               return false;
            if (trigger.buttons[0].gamepad != xinput_button::none)
               return false;
            return true;
         }
         return false;
      }
   };
}