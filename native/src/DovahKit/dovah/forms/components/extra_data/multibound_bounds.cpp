#include "multibound_bounds.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result multibound_bounds::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->halfwidths.x);
      subrecord.read(this->halfwidths.y);
      subrecord.read(this->halfwidths.z);
      return load_result::succeeded;
   }
   void multibound_bounds::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->halfwidths.x);
      subrecord.write(this->halfwidths.y);
      subrecord.write(this->halfwidths.z);
      subrecord.close();
   }
}