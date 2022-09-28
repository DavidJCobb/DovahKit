#include "hkWorldObjCinfoProperty.h"
#include "../reader.h"

namespace nifDK {
   void hkWorldObjCinfoProperty::read(file_reader& reader) {
      reader.require_size(sizeof(uint32_t) * 3);
      reader.unchecked_read(*this);
   }
   void hkWorldObjCinfoProperty::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->data);
      reader.unchecked_read(this->size);
      reader.unchecked_read(this->capacity_and_flags);
   }
}