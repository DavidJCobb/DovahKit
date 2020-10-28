#include "portal.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result portal::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->width);
      subrecord.read(this->height);
      subrecord.read(this->position.x);
      subrecord.read(this->position.y);
      subrecord.read(this->position.z);
      subrecord.read(this->rotation.a);
      subrecord.read(this->rotation.b);
      subrecord.read(this->rotation.c);
      subrecord.read(this->rotation.d);
      return load_result::succeeded;
   }
   void portal::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->width);
      subrecord.write(this->height);
      subrecord.write(this->position.x);
      subrecord.write(this->position.y);
      subrecord.write(this->position.z);
      subrecord.write(this->rotation.a);
      subrecord.write(this->rotation.b);
      subrecord.write(this->rotation.c);
      subrecord.write(this->rotation.d);
      subrecord.close();
   }
   basic_extra_data* portal::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new portal;
      clone->width    = this->width;
      clone->height   = this->height;
      clone->position = this->position;
      clone->rotation = this->rotation;
      return clone;
   }
}