#pragma once
#include <Qt>
#include "editor/subsystems/xinput/enums/button.h"
#include "editor/subsystems/xinput/gamepad.h"
#include "../enums/button_press_type.h"
#include "../chrono.h"
#include "../device_button_state.h"

#include "./components/buttoned_device.h"

namespace dovahkit::subsystems::worldinput2 {
   namespace inputs {
      struct button;
   }
}

namespace dovahkit::subsystems::worldinput2::devices {
   class xinput {
      public:
         using button = subsystems::xinput::button;
         static constexpr size_t button_count = 16;

      public:
         bool is_connected = false;
         components::buttoned_device<button_count> buttons;
         struct {
            QPointF ls = {};
            QPointF rs = {};
         } vectors;
         struct {
            float lt = 0;
            float rt = 0;
         } scalars;

         void ignore_all_down();
         void update(timestamp_t now, bool connected, const subsystems::xinput::gamepad&);

         button_press_type release_type(const inputs::button&) const;
         bool is_down(const inputs::button&) const;
         timestamp_t down_when(const inputs::button&) const;
         device_button_state key_down_state(const inputs::button&) const;
   };
}