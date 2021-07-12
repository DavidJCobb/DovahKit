#include "decal_data.h"

namespace dovah::loaded_forms::components {
   void decal_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      subrecord.read(this->width.min);
      subrecord.read(this->width.max);
      subrecord.read(this->height.min);
      subrecord.read(this->height.max);
      subrecord.read(this->depth);
      subrecord.read(this->shininess);
      subrecord.read(this->parallax.scale);
      subrecord.read(this->parallax.passes);
      subrecord.read(this->flags);
      subrecord.skip_bytes(2);
      this->color.load(subrecord);
   }
   void decal_data::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.reserve_more(0x24);
      subrecord.write(this->width.min);
      subrecord.write(this->width.max);
      subrecord.write(this->height.min);
      subrecord.write(this->height.max);
      subrecord.write(this->depth);
      subrecord.write(this->shininess);
      subrecord.write(this->parallax.scale);
      subrecord.write(this->parallax.passes);
      subrecord.write(this->flags);
      subrecord.skip_bytes(2);
      this->color.save(subrecord);
   }
}
