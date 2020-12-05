#include "poison.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result poison::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case signature_type:
            subrecord.read(this->type);
            intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::potion, intfc.target_stub, this->type) // TODO: verify form type
            );
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
      if (!this->type)
         return;
      record.write_formID_subrecord(signature_type, this->type);
      auto& XPSC = record.open_next_subrecord(signature_dose);
      XPSC.write(this->doses);
      XPSC.close();
   }
   //
   /*static*/ void poison::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_type) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            uib.add_outbound_reference(formID);
      }
   }
   basic_extra_data* poison::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new poison;
      clone->type.set(clone_owner, this->type);
      clone->doses = this->doses;
      return clone;
   }
   void poison::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      this->type.clear_if(my_owner, target);
   }
}