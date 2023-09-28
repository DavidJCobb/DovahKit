#pragma once
#include "orbit_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr bool orbit_camera::options::operator==(const options& other) const noexcept {
      if (this->magnitudes != other.magnitudes)
         return false;
      if (this->range.x != other.range.x)
         return false;
      if (this->range.y != other.range.y)
         return false;
      if (this->target != other.target)
         return false;
      return true;
   }

   constexpr void orbit_camera::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         magnitudes.yaw,
         magnitudes.pitch,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign,
         target
      );
   }
   constexpr void orbit_camera::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         magnitudes.yaw,
         magnitudes.pitch,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign,
         target
      );
   }
   static_assert(cobb::bitstreams::round_trip_test<orbit_camera::options>, "Assert: round-trip bitstream serialization produces correct results.");

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