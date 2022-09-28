#include "bhkVelocityConstraintMotor.h"
#include "../reader.h"

namespace nifDK {
   void bhkVelocityConstraintMotor::read(file_reader& reader) {
      reader.read(this->force.minimum);
      reader.read(this->force.maximum);
      reader.read(this->tau);
      reader.read(this->target_velocity);
      reader.read(this->use_target_velocity);
      reader.read(this->enabled);
   }
}