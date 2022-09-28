#include "bhkSpringDamperConstraintMotor.h"
#include "../reader.h"

namespace nifDK {
   void bhkSpringDamperConstraintMotor::read(file_reader& reader) {
      reader.read(this->force.minimum);
      reader.read(this->force.maximum);
      reader.read(this->spring.constant);
      reader.read(this->spring.damping);
      reader.read(this->enabled);
   }
}