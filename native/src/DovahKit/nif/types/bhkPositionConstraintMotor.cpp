#include "bhkPositionConstraintMotor.h"
#include "../reader.h"

namespace nifDK {
   void bhkPositionConstraintMotor::read(file_reader& reader) {
      reader.read(this->force.minimum);
      reader.read(this->force.maximum);
      reader.read(this->tau);
      reader.read(this->damping);
      reader.read(this->recovery_velocity.proportional);
      reader.read(this->recovery_velocity.constant);
      reader.read(this->enabled);
   }
}