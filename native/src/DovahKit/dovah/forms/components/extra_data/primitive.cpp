#include "primitive.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result primitive::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->bounds.x);
      subrecord.read(this->bounds.y);
      subrecord.read(this->bounds.z);
      subrecord.read(this->color.r);
      subrecord.read(this->color.g);
      subrecord.read(this->color.b);
      subrecord.read(this->unknown);
      subrecord.read(this->type);
      return load_result::succeeded;
   }
   void primitive::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->bounds.x);
      subrecord.write(this->bounds.y);
      subrecord.write(this->bounds.z);
      subrecord.write(this->color.r);
      subrecord.write(this->color.g);
      subrecord.write(this->color.b);
      subrecord.write(this->unknown);
      subrecord.write(this->type);
      subrecord.close();
   }
}