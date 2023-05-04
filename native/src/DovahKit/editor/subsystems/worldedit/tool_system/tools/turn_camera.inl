#pragma once
#include "turn_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
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