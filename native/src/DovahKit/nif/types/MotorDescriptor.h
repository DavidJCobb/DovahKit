#pragma once
#include <variant>
#include "./bhkPositionConstraintMotor.h"
#include "./bhkSpringDamperConstraintMotor.h"
#include "./bhkVelocityConstraintMotor.h"

namespace nifDK {
   class file_reader;

   struct MotorDescriptor {
      std::variant<
         std::monostate,
         bhkPositionConstraintMotor,
         bhkVelocityConstraintMotor,
         bhkSpringDamperConstraintMotor
      > data;

      void read(file_reader&);
   };
}