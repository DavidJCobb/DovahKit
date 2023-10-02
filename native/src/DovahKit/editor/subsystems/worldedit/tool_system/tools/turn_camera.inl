#pragma once
#include "turn_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void turn_camera::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         magnitudes.x,
         magnitudes.y,
         magnitudes.z
      );
      {  // range
         bool presence = false;
         s.stream(presence);
         if (presence) {
            this->range.emplace();
            s.stream(this->range.value());
         }
      }
   }
   constexpr void turn_camera::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         magnitudes.x,
         magnitudes.y,
         magnitudes.z
      );

      s.stream(range.has_value());
      if (range.has_value())
         s.stream(range.value());
   }
   static_assert(
      cobb::bitstreams::round_trip_test_with_targeted_scramble<
         turn_camera::options,
         //
         // MSVC cannot perform a bitcast on a std::optional, which is understandable, frankly. 
         // We need to use the "targeted scramble" test and explicitly specify which members 
         // are safe to scramble as part of the test procedure:
         //
         &turn_camera::options::magnitudes
      >(),
      "Assert: round-trip bitstream serialization produces correct results."
   );

   constexpr void turn_camera::response::scale(double delta_seconds) {
      this->held *= delta_seconds;
   }
   constexpr void turn_camera::response::merge(const response& from) {
      this->held    += from.held;
      this->instant += from.instant;
   }
}