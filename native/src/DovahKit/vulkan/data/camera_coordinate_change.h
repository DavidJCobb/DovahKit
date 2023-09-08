#pragma once
#include <glm/glm.hpp>

namespace vulkanDK::data {
   struct camera_coordinate_change {
      glm::vec3 move = { 0, 0, 0 }; // camera-relative, assuming Y-forward and Z-up (as if the camera were a REFR)
      glm::vec3 turn = { 0, 0, 0 }; // x, y, z == pitch, roll, yaw; camera-relative
   };
}