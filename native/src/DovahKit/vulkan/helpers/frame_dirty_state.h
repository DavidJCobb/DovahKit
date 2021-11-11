#pragma once
#include <cstdint>

namespace vulkanDK {
   //
   // Range of frames that we've already done updates for.
   //
   class frame_dirty_state {
      protected:
         uint16_t start = 0;
         uint16_t end   = 0;
      public:
         inline bool is_up_to_date(uint16_t frame_index) const noexcept { return frame_index >= start && frame_index < end; }
         void set_up_to_date(uint16_t frame_index) {
            if (frame_index == this->end) {
               ++this->end;
            } else {
               this->start = frame_index;
               this->end   = frame_index + 1;
            }
         }

         inline void set_all_out_of_date() {
            this->start = this->end = 0;
         }
         bool are_all_up_to_date(uint16_t frame_count) const noexcept {
            return (this->start == 0) && (this->end == frame_count);
         }

         void set_all_up_to_date(uint16_t frame_count) {
            this->start = 0;
            this->end   = frame_count;
         }
   };
}