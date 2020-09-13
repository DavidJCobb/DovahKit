#include "bounds.h"

namespace dovah::loaded_forms::components {
   void object_bounds::load(tes_subrecord_reader& subrecord) {
      if (!subrecord.is_in_bounds(12))
         return;
      subrecord.unchecked_read(this->min.x);
      subrecord.unchecked_read(this->min.y);
      subrecord.unchecked_read(this->min.z);
      subrecord.unchecked_read(this->max.x);
      subrecord.unchecked_read(this->max.y);
      subrecord.unchecked_read(this->max.z);
   }
   /*static*/ void object_bounds::generateUseInfo(tes_subrecord_reader&, form_stub*) {
      return; // no use info to generate
   }
}
