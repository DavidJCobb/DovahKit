#pragma once
#include <array>
#include <bitset>
#include <Qt>
#include "helpers/bitfield_array.h"
#include "editor/subsystems/DKXInputSubsystem.h"
#include "dk3d/chrono.h"
#include "dk3d/InputResult.h"
#include "dk3d/KeyDownState.h"

namespace DK3D {
   namespace inputs {
      struct button;
   }
}

namespace DK3D::devices {
   class xinput {
      public:
         using Button = DKXInputSubsystem::Button;
         static constexpr size_t button_count = 16;

      public:
         bool is_connected = false;
         struct {
            std::array<timestamp_t, button_count> start;
            std::bitset<button_count> ignore;
            cobb::bitfield_array<KeyReleaseType, button_count, 2> releases;
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
         void update(timestamp_t now, bool connected, const DKXInputSubsystem::Gamepad&);

         void mark_button_processed(const inputs::button&);
         bool is_button_processed(const inputs::button&) const;

         KeyReleaseType release_type(const inputs::button&) const;
         bool is_down(const inputs::button&, bool even_if_ignored = false) const;
         timestamp_t down_when(const inputs::button&) const;
         KeyDownState key_down_state(const inputs::button&) const;
   };
}