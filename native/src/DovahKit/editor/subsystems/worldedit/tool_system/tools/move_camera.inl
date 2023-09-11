#pragma once
#include "./move_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void move_camera::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         reference_frames.baseline,
         reference_frames.selection,
         magnitudes.x,
         magnitudes.y,
         magnitudes.z,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign
      );
   }
   constexpr void move_camera::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         reference_frames.baseline,
         reference_frames.selection,
         magnitudes.x,
         magnitudes.y,
         magnitudes.z,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign
      );
   }
   static_assert(cobb::bitstreams::round_trip_test<move_camera::options>, "Assert: round-trip bitstream serialization produces correct results.");

   constexpr void move_camera::response::scale(double delta_seconds) {
      this->held *= delta_seconds;
   }
   constexpr void move_camera::response::merge(const response& from) {
      this->held    += from.held;
      this->instant += from.instant;
   }
}