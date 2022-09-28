#pragma once
#include <glm/glm.hpp>
#include "./MotorDescriptor.h"

namespace nifDK {
   class file_reader;

   struct PrismaticDescriptor {
      glm::fmat4x4 transform_a;
      glm::fmat4x4 transform_b;
      struct {
         float minimum;
         float maximum;
      } distance;
      float friction;
      MotorDescriptor motor;

      void read(file_reader&);
   };
}