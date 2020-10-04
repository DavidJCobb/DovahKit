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
   //
   /*static*/ void ammo::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_type) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            stub->add_outbound_reference(formID);
      }
   }
   basic_extra_data* ammo::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new ammo;
      clone->type.set(&clone_owner, this->type);
      clone->count = this->count;
      return clone;
   }
   void ammo::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      if (this->type == target.formID)
         this->type.set(&my_owner, nullptr);
   }
}