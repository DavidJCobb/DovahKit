#include "bhkLimitedHingeConstraint.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkLimitedHingeConstraint::parse(file_reader& reader) {
      reader.read(this->data);
   }
}