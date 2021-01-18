#include "linked_ref.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result linked_ref::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      if (subrecord.size() >= 8) {
         subrecord.read(this->keyword);
         intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
            detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::keyword, intfc.target_stub, this->keyword)
         );
      }
      subrecord.read(this->ref);
      intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
         detailed_notice::warn_if_not_object_reference(subrecord.signature(), intfc.target_stub, this->ref)
      );
      return load_result::succeeded;
   }
   void linked_ref::save(tes_record_writer& record, save_interface_t& intfc) {
      if (!this->ref)
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      if (this->keyword)
         subrecord.write(this->keyword);
      subrecord.write(this->ref);
      subrecord.close();
   }
   //
   /*static*/ void linked_ref::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         uib.add_outbound_reference(formID);
      if (!subrecord.is_in_bounds(4))
         return;
      if (subrecord.read(formID) && formID)
         uib.add_outbound_reference(formID);
   }
   basic_extra_data* linked_ref::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new linked_ref;
      clone->keyword.set(clone_owner, this->keyword);
      clone->ref.set(clone_owner, this->ref);
      return clone;
   }
   void linked_ref::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      this->keyword.clear_if(my_owner, target);
      this->ref.clear_if(my_owner, target);
   }
}