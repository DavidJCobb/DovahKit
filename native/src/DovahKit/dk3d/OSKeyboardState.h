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
         //
         struct {
            QPoint pos;  // mouse screen position; tracked so we can generate the (move) field
            QPoint move; // distance the mouse moved since the last update
         } mouse;
         struct {
            //
            // Fields relating to system configuration, e.g. whether the left and right mouse buttons 
            // have been swapped.
            //
            struct {
               bool has_scroll_wheel = false;
               bool swap_left_right  = false;
               struct {
                  QPoint double_click; // second click must occur within this distance of the first
                  QPoint drag;         // minimum mousemove distance before a click becomes a drag
               } hitboxes;
            } mouse;
         } system;

         void ignoreAllDown();
         void update(timestamp_t now);

         void recheckMouseMetrics();

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