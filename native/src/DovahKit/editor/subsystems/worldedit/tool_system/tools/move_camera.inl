#pragma once
#include "./move_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void move_camera::results::scale(double delta_seconds) {
      this->x *= delta_seconds;
      this->y *= delta_seconds;
      this->z *= delta_seconds;
   }
   constexpr void move_camera::results::merge(const results& from) {
      this->x += from.x;
      this->y += from.y;
      this->z += from.z;
   }
}