#include "poison.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result poison::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case signature_type:
            subrecord.read(this->type);
            break;
         case signature_dose:
            subrecord.read(this->doses);
            break;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void poison::save(tes_record_writer& record) {
      //
      // NOTE: We should write our subrecords even if the values match the defaults, 
      // because we can't tell whether we're writing to a new form or to an override. 
      // If the user wants to overwrite a record that has non-default poison settings 
      // in order to return them to their defaults, then we need to make sure that 
      // that works.
      //
      record.write_formID_subrecord(signature_type, this->type);
      auto& XPSC = record.open_next_subrecord(signature_dose);
      XPSC.write(this->doses);
      XPSC.close();
   }
   //
   /*static*/ void poison::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_type) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            stub->add_outbound_reference(formID);
      }
   }
}