#include "portal_origin_and_destination.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result portal_origin_and_destination::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->origin);
      subrecord.read(this->destination);
      intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
         detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, intfc.target_stub, this->origin)
      );
      intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
         detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, intfc.target_stub, this->destination)
      );
      return load_result::succeeded;
   }
   void portal_origin_and_destination::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->origin);
      subrecord.write(this->destination);
      subrecord.close();
   }
   //
   /*static*/ void portal_origin_and_destination::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         uib.add_outbound_reference(formID);
      if (subrecord.read(formID) && formID)
         uib.add_outbound_reference(formID);
   }
   basic_extra_data* portal_origin_and_destination::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new portal_origin_and_destination;
      clone->origin.set(clone_owner, this->origin);
      clone->destination.set(clone_owner, this->destination);
      return clone;
   }
   void portal_origin_and_destination::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      this->origin.clear_if(my_owner, target);
      this->destination.clear_if(my_owner, target);
   }
}