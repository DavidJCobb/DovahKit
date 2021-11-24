#pragma once
#include <array>
#include <bitset>
#include <Qt>
#include "../helpers/bitfield_array.h"
#include "chrono.h"
#include "InputResult.h"
#include "KeyDownState.h"

namespace DK3D {
   class OSKeyboardState { // also handles mouse buttons
      public:
         static constexpr size_t vk_code_count = 256;

      public:
         std::array<timestamp_t, vk_code_count> start;
         std::bitset<vk_code_count> ignore;
         cobb::bitfield_array<KeyReleaseType, vk_code_count, 2> releases;

         void ignoreAllDown();
         void update(timestamp_t now);

         KeyReleaseType releaseType(int vk) const;
         KeyReleaseType releaseType(Qt::MouseButton) const;
         bool isDown(int vk, bool even_if_ignored = false) const;
         bool isDown(Qt::MouseButton, bool even_if_ignored = false) const;
         timestamp_t downWhen(int vk) const;
         timestamp_t downWhen(Qt::MouseButton) const;
         KeyDownState keyDownState(int vk) const;
         KeyDownState keyDownState(Qt::MouseButton) const;
   };
}