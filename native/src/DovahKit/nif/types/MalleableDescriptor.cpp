#include "MalleableDescriptor.h"
#include "../reader.h"

namespace nifDK {
   void MalleableDescriptor::read(file_reader& reader) {
      ConstraintData::read(reader);

      reader.read(this->tau);
      reader.read(this->damping);
      reader.read(this->strength);
   }
}