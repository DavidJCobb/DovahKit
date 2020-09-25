#include "patrol_ref_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result patrol_ref_data::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case signature_time:
            subrecord.read(this->idle_time);
            break;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void patrol_ref_data::save(tes_record_writer& record) {
      auto& XPRD = record.open_next_subrecord(signature_time);
      XPRD.write(this->idle_time);
      XPRD.close();
   }
}