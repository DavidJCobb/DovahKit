#pragma once
#include "turn_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void turn_camera::options::read(options_serialization_version version, cobb::streams::bitreader& stream) {
      stream.read(
         magnitudes.yaw,
         magnitudes.pitch,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign
      );
   }
   constexpr void turn_camera::options::write(cobb::streams::bitwriter& stream) const {
      stream.write(
         magnitudes.yaw,
         magnitudes.pitch,
         range.x.axis,
         range.x.sign,
         range.y.axis,
         range.y.sign
      );
   }

   constexpr void turn_camera::results::scale(double delta_seconds) {
      this->yaw *= delta_seconds;
      this->pitch *= delta_seconds;
      this->roll *= delta_seconds;
   }
   constexpr void turn_camera::results::merge(const results& from) {
      this->yaw += from.yaw;
      this->pitch += from.pitch;
      this->roll += from.roll;
   }
}