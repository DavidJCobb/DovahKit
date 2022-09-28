#pragma once
#include <glm/glm.hpp>
#include "./MotorDescriptor.h"

namespace nifDK {
   class file_reader;

   struct RagdollDescriptor {
      glm::fmat4x4 transform_a;
      glm::fmat4x4 transform_b;
      float cone_max_angle;
      struct {
         float minimum;
         float maximum;
      } plane_angle;
      struct {
         float minimum;
         float maximum;
      } twist_angle;
      float max_friction;
      MotorDescriptor motor;

      void read(file_reader&);
   };
}