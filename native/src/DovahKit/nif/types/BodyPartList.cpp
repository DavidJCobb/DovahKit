#include "BodyPartList.h"
#include "../reader.h"

namespace nifDK {
   void BodyPartList::read(file_reader& reader) {
      reader.require_size(4);
      this->unchecked_read(reader);
   }
   void BodyPartList::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->flags);
      reader.unchecked_read(this->parts);
   }
}