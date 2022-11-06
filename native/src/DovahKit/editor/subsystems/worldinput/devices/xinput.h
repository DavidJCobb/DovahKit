#pragma once
#include <array>
#include <bitset>
#include <Qt>
#include "helpers/bitfield_array.h"
#include "editor/subsystems/xinput/enums/button.h"
#include "editor/subsystems/xinput/gamepad.h"
#include "../enums/button_release_type.h"
#include "../chrono.h"
#include "../button_state.h"

namespace dovahkit::subsystems::worldinput {
   namespace inputs {
      struct button;
   }
}

namespace dovahkit::subsystems::worldinput::devices {
   class xinput {
      public:
         using button = subsystems::xinput::button;
         static constexpr size_t button_count = 16;

      public:
         bool is_connected = false;
         struct {
            std::array<timestamp_t, button_count> start;
            std::bitset<button_count> ignore;
            cobb::bitfield_array<button_release_type, button_count, 2> releases;
            //
            std::bitset<button_count> processed;
         } buttons;
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

         void mark_button_processed(const inputs::button&);
         bool is_button_processed(const inputs::button&) const;

         button_release_type release_type(const inputs::button&) const;
         bool is_down(const inputs::button&, bool even_if_ignored = false) const;
         timestamp_t down_when(const inputs::button&) const;
         button_state key_down_state(const inputs::button&) const;
   };
}