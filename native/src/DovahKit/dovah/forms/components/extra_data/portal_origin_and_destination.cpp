#include "portal_origin_and_destination.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result portal_origin_and_destination::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->origin);
      subrecord.read(this->destination);
      return load_result::succeeded;
   }
   void portal_origin_and_destination::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->origin);
      subrecord.write(this->destination);
      subrecord.close();
   }
}