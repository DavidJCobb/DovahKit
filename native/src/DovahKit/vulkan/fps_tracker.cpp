#include "fps_tracker.h"
#include <algorithm> // std::min

namespace vulkanDK {
   void fps_tracker::next_delta(delta_type delta) {
      constexpr auto one_second = delta_type(1);
      //
      if (delta == delta_type(0)) {
         //
         // Instantaneous frame; dividing would be a  division by zero. Refer to documentation on 
         // how we measure FPS, but basically, it's best to just skip measuring this frame.
         //
         return;
      }
      value_type fps_value = one_second / delta;
      //
      if constexpr (mode == counting_mode::count) {
         //
         // Count the number of frames that render during a single second; update the 
         // average every second.
         //
         auto& h = this->history;
         if (h.count == 0) {
            h.total = delta;
            h.count = 1;
         } else {
            h.total += delta;
            ++h.count;
            if (h.total >= one_second) {
               this->value = value_type(h.count); // alternatively: value_type(delta_type(h.count) / h.total);
               //
               h.total = delta_type(0);
               h.count = 0;
            }
         }
      } else if constexpr (mode == counting_mode::average) {
         //
         // Take a rolling average of the frames-per-second, updated every frame. The 
         // problem with this method is that if we have 30 seconds at 400 FPS (because 
         // the scene is empty), and then the frame rate lowers to 20 FPS (because the 
         // scene has loaded and the current view is expensive), it will take a very 
         // long time for the FPS counter to settle back down from 400 to 20.
         //
         auto& h = this->history;
         if (h.count == 0) {
            h.total = fps_value;
            h.count = 1;
         } else if (h.count == std::numeric_limits<decltype(h.count)>::max() - 1) { // overflow imminent
            if (fps_value != max_value) {
               h.total = fps_value;
            }
            h.count = 1;
         } else {
            using average_t = decltype(h.total);
            //
            fps_value = std::min(fps_value, max_visible_value);
            //
            constexpr bool alternate_method = true;
            if constexpr (alternate_method) {
               ++h.count;
               h.total += ((average_t)fps_value - h.total) / h.count;
            } else {
               h.total = ((average_t)fps_value + (average_t)h.count * h.total) / (h.count + 1);
               ++h.count;
            }
         }
      } else {
         //
         // Just compute the frame rate from the current delta alone and display 
         // it verbatim.
         //
         if (this->value == fps_value)
            return;
         this->value = fps_value;
      }
   }
   fps_tracker::value_type fps_tracker::display_value() const {
      if constexpr (mode == counting_mode::average)
         return this->history.total;
      return this->value;
   }
}