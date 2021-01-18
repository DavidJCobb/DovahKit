#include "ammo.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result ammo::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      switch (subrecord.signature()) {
         case signature_type:
            subrecord.read(this->type);
            intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::ammo, intfc.target_stub, this->type)
            );
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
   void ammo::save(tes_record_writer& record, save_interface_t& intfc) {
      if (!this->type)
         return;
      record.write_formID_subrecord(signature_type, this->type);
      auto& XPSC = record.open_next_subrecord(signature_count);
      XPSC.write(this->count);
      XPSC.close();
   }
   //
   /*static*/ void ammo::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_type) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            uib.add_outbound_reference(formID);
      }
   }
   basic_extra_data* ammo::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new ammo;
      clone->type.set(clone_owner, this->type);
      clone->count = this->count;
      return clone;
   }
   void ammo::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      this->type.clear_if(my_owner, target);
   }
}