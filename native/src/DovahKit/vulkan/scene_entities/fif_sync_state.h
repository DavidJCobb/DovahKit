#pragma once
#include <cstdint>
#include <type_traits>
#include "../config/frames_in_flight.h"

namespace vulkanDK::scene_entities {
   //
   // Range of frames that we've already done updates for.
   //
   class fif_sync_state {
      protected:
         static constexpr size_t bitcount = config::frames_in_flight_count;
         using mask_type = std::conditional_t<
            (bitcount <= 8),
            uint32_t,
            std::conditional_t<
               (bitcount <= 16),
               uint16_t,
               uint32_t
            >
         >;

         static constexpr mask_type all_set = mask_type(1 << bitcount) - 1;

      protected:
         mask_type mask = 0;

      public:
         static constexpr mask_type mask_for(uint8_t frame_index) {
            return mask_type(1) << frame_index;
         }

         constexpr bool is_up_to_date(uint16_t frame_index) const noexcept { return (mask & mask_for(frame_index)) != 0; }
         constexpr void set_up_to_date(uint16_t frame_index) {
            mask |= mask_for(frame_index);
         }
         constexpr void set_out_of_date(uint16_t frame_index) {
            mask &= ~mask_for(frame_index);
         }

         constexpr void set_all_out_of_date() {
            mask = 0;
         }
         constexpr bool are_all_up_to_date() const noexcept {
            return (mask & all_set) == all_set;
         }

         constexpr void set_all_up_to_date() {
            mask = all_set;
         }
   };
}