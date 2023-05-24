#pragma once
#include "turn_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void turn_camera::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         magnitudes.yaw,
         magnitudes.pitch,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign
      );
   }
   constexpr void turn_camera::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         magnitudes.yaw,
         magnitudes.pitch,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign
      );
   }
   static_assert(cobb::bitstreams::round_trip_test<turn_camera::options>, "Assert: round-trip bitstream serialization produces correct results.");

   constexpr void turn_camera::results::scale(double delta_seconds) {
      this->yaw   *= delta_seconds;
      this->pitch *= delta_seconds;
      this->roll  *= delta_seconds;
   }
   constexpr void turn_camera::results::merge(const results& from) {
      this->yaw   += from.yaw;
      this->pitch += from.pitch;
      this->roll  += from.roll;
   }
}