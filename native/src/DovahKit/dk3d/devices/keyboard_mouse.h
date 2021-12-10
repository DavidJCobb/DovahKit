#pragma once
#include <array>
#include <bitset>
#include <Qt>
#include "helpers/bitfield_array.h"
#include "dk3d/chrono.h"
#include "dk3d/InputResult.h"
#include "dk3d/KeyDownState.h"

namespace DK3D {
   namespace inputs {
      struct button;
   }
}

namespace DK3D::devices {
   class keyboard_mouse {
      public:
         static constexpr size_t vk_code_count = 256;

      public:
         struct {
            std::array<timestamp_t, vk_code_count> start;
            std::bitset<vk_code_count> ignore;
            cobb::bitfield_array<KeyReleaseType, vk_code_count, 2> releases;
            //
            std::bitset<vk_code_count> processed;
         } buttons;
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
               uint double_click_ms  = 500;
               struct {
                  QPoint double_click; // second click must occur within this distance of the first
                  QPoint drag;         // minimum mousemove distance before a click becomes a drag
               } hitboxes;
            } mouse;
         } system;

      public:
         void recheck_mouse_metrics();

         void ignore_all_down();
         void update(timestamp_t now);

         void mark_button_processed(const inputs::button&);
         bool is_button_processed(const inputs::button&) const;

         KeyReleaseType release_type(const inputs::button&) const;
         bool is_down(const inputs::button&, bool even_if_ignored = false) const;
         timestamp_t down_when(const inputs::button&) const;
         KeyDownState key_down_state(const inputs::button&) const;
   };
}