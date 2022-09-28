#include "MotorDescriptor.h"
#include <cstdint>
#include "../reader.h"
#include "../notice_code_list.h"

namespace nifDK {
   void MotorDescriptor::read(file_reader& reader) {
      uint8_t type;
      reader.read(type);

      switch (type) {
         case 0: // position
            reader.read(this->data = bhkPositionConstraintMotor{});
            break;
         case 1: // velocity
            reader.read(this->data = bhkVelocityConstraintMotor{});
            break;
         case 2: // spring damper
            reader.read(this->data = bhkSpringDamperConstraintMotor{});
            break;
         case 3:
            reader.throw_error(notice_code::havok_motor_invalid_type);
      }
   }
}