#pragma once
#include <chrono>

namespace dovahkit::subsystems::worldinput {
   using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
   inline constexpr timestamp_t zero_timestamp = timestamp_t(timestamp_t::duration::zero());

   inline timestamp_t current_time() {
      return std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
   }
   constexpr auto elapsed_time(timestamp_t start, timestamp_t now) {
      if (start == zero_timestamp)
         return (1.0 / 60.0);
      return std::chrono::duration<double, std::chrono::seconds::period>(now - start).count();
   }
}