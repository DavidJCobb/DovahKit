#include "color_floats.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms {
   bool color_floats::load(tes_subrecord_reader& subrecord) {
      if (subrecord.is_in_bounds(4)) {
         subrecord.unchecked_read(this->r);
         subrecord.unchecked_read(this->g);
         subrecord.unchecked_read(this->b);
         subrecord.unchecked_read(this->a);
         return true;
      }
      return false;
   }
   void color_floats::save(tes_subrecord_writer& subrecord) {
      subrecord.write(this->r);
      subrecord.write(this->g);
      subrecord.write(this->b);
      subrecord.write(this->a);
   }
}