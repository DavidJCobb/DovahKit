#include "ConstraintData.h"
#include "../reader.h"

namespace nifDK {
   void ConstraintData::read(file_reader& reader) {
      hkConstraintType type;
      reader.read(type);

      uint32_t count;
      reader.read(count);
      if (count > 0) {
         reader.read_ref(this->target_a);
         if (count > 1) {
            reader.read_ref(this->target_b);
         }
      }

      reader.read(this->priority);
      
      this->data.read_data(reader, type);
   }
}