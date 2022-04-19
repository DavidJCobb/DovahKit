#pragma once
#include <cstdint>
#include <type_traits>
#include "../config/frames_in_flight.h"

namespace vulkanDK {
   //
   // Range of frames that we've already done updates for.
   //
   class frame_dirty_state {
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
         inline bool is_up_to_date(uint16_t frame_index) const noexcept { return (mask & (mask_type(1) << frame_index)) != 0; }
         void set_up_to_date(uint16_t frame_index) {
            mask |= mask_type(1) << frame_index;
         }

         inline void set_all_out_of_date() {
            mask = 0;
         }
         bool are_all_up_to_date() const noexcept {
            return (mask & all_set) == all_set;
         }

         void set_all_up_to_date() {
            mask = all_set;
         }
   };
}