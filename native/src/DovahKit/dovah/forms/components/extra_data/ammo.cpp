#include "ammo.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result ammo::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case signature_type:
            subrecord.read(this->type);
            this->count = 999;
            break;
         case signature_count:
            subrecord.read(this->count);
            break;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void ammo::save(tes_record_writer& record) {
      //
      // NOTE: We should write our subrecords even if the values match the defaults, 
      // because we can't tell whether we're writing to a new form or to an override. 
      // If the user wants to overwrite a record that has non-default poison settings 
      // in order to return them to their defaults, then we need to make sure that 
      // that works.
      //
      record.write_formID_subrecord(signature_type, this->type);
      auto& XPSC = record.open_next_subrecord(signature_count);
      XPSC.write(this->count);
      XPSC.close();
   }
}