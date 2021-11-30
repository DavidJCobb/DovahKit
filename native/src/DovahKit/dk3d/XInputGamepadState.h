#pragma once
#include <array>
#include <bitset>
#include <Qt>
#include "../helpers/bitfield_array.h"
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "chrono.h"
#include "InputResult.h"
#include "KeyDownState.h"

namespace DK3D {
   class XInputGamepadState {
      public:
         using Button = DKXInputSubsystem::Button;
         static constexpr size_t button_count = 16;

      public:
         bool is_connected = false;
         struct {
            std::array<timestamp_t, button_count> start;
            std::bitset<button_count> ignore;
            cobb::bitfield_array<KeyReleaseType, button_count, 2> releases;
         } buttons;
         struct {
            QPointF ls = {};
            QPointF rs = {};
         } vectors;
         struct {
            float lt = 0;
            float rt = 0;
         } scalars;

         void ignoreAllDown();
         void update(timestamp_t now, bool connected, const DKXInputSubsystem::Gamepad&);

         KeyReleaseType releaseType(Button) const;
         bool isDown(Button, bool even_if_ignored = false) const;
         timestamp_t downWhen(Button) const;
         KeyDownState keyDownState(Button) const;
   };
}