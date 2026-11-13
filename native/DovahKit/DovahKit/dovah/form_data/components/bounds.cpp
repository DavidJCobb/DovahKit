#include "./bounds.h"
#include "dovah/tes/reading/subrecord_view.h"

namespace dovah::form_data::components {
   bool bounds::stream(tes::subrecord_view& subrecord) {
      return subrecord.read(
         this->min.x,
         this->min.y,
         this->min.z,
         this->max.x,
         this->max.y,
         this->max.z
      );
   }
   void bounds::unchecked_stream(tes::subrecord_view& subrecord) {
      subrecord.unchecked_read(
         this->min.x,
         this->min.y,
         this->min.z,
         this->max.x,
         this->max.y,
         this->max.z
      );
   }
}