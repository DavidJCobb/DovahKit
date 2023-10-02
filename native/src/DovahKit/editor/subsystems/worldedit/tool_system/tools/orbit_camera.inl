#pragma once
#include "orbit_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr bool orbit_camera::options::operator==(const options& other) const noexcept {
      if (this->magnitudes != other.magnitudes)
         return false;
      if (this->range != other.range)
         return false;
      if (this->target != other.target)
         return false;
      return true;
   }

   constexpr void orbit_camera::options::stream(cobb::bitstreams::reader& s) {
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

      s.stream(target);
   }
   constexpr void orbit_camera::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         magnitudes.x,
         magnitudes.y,
         magnitudes.z
      );

      s.stream(range.has_value());
      if (range.has_value())
         s.stream(range.value());

      s.stream(target);
   }
   static_assert(
      cobb::bitstreams::round_trip_test_with_targeted_scramble<
         orbit_camera::options,
         //
         // MSVC cannot perform a bitcast on a std::optional, which is understandable, frankly. 
         // We need to use the "targeted scramble" test and explicitly specify which members 
         // are safe to scramble as part of the test procedure:
         //
         &orbit_camera::options::magnitudes,
         &orbit_camera::options::target
      >(),
      "Assert: round-trip bitstream serialization produces correct results."
   );

   constexpr void orbit_camera::response::scale(double delta_seconds) {
      this->held *= delta_seconds;
   }
   constexpr void orbit_camera::response::merge(const response& from) {
      // NOTE: Of the two `response` structs at play here, the argument is the newer/higher-priority 
      // one and `this` is the older/lower-priority one.
      if (this->target != from.target) {
         *this = from;
         return;
      }
      this->held    += from.held;
      this->instant += from.instant;
   }
}