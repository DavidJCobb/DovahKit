#include "teleport.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result teleport::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->target_door);
      subrecord.read(this->position.x);
      subrecord.read(this->position.y);
      subrecord.read(this->position.z);
      subrecord.read(this->rotation.x);
      subrecord.read(this->rotation.y);
      subrecord.read(this->rotation.z);
      subrecord.read(this->flags);
      return load_result::succeeded;
   }
   void teleport::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->target_door);
      subrecord.write(this->position.x);
      subrecord.write(this->position.y);
      subrecord.write(this->position.z);
      subrecord.write(this->rotation.x);
      subrecord.write(this->rotation.y);
      subrecord.write(this->rotation.z);
      subrecord.write(this->flags);
      subrecord.close();
   }
}