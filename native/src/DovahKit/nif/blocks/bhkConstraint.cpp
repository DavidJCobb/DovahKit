#include "bhkConstraint.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkConstraint::parse(file_reader& reader) {
      bhkEntity::parse(reader);

      uint32_t count;

      reader.read(count);
      this->subjects.resize(count);
      for (auto& item : this->subjects)
         reader.read_ref(item);

      reader.read(this->priority);
   }
}