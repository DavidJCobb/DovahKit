#include "bounds.h"

namespace dovah::loaded_forms::components {
   void object_bounds::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (!subrecord.is_in_bounds(12))
         return;
      subrecord.unchecked_read(this->min.x);
      subrecord.unchecked_read(this->min.y);
      subrecord.unchecked_read(this->min.z);
      subrecord.unchecked_read(this->max.x);
      subrecord.unchecked_read(this->max.y);
      subrecord.unchecked_read(this->max.z);
   }
   /*static*/ void object_bounds::generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder& uib) {
      return; // no use info to generate
   }
   void object_bounds::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.reserve_more(12);
      subrecord.write(this->min.x);
      subrecord.write(this->min.y);
      subrecord.write(this->min.z);
      subrecord.write(this->max.x);
      subrecord.write(this->max.y);
      subrecord.write(this->max.z);
   }
}
