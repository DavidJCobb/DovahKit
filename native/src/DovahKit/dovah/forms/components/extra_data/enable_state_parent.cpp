#include "enable_state_parent.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result enable_state_parent::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->ref);
      subrecord.read(this->flags);
      subrecord.read(this->pad05);
      return load_result::succeeded;
   }
   void enable_state_parent::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->ref);
      subrecord.write(this->flags);
      subrecord.write(this->pad05);
      subrecord.close();
   }
   //
   /*static*/ void enable_state_parent::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            uib.add_outbound_reference(formID);
      }
   }
   basic_extra_data* enable_state_parent::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new enable_state_parent;
      clone->ref.set(clone_owner, this->ref);
      clone->flags = this->flags;
      clone->pad05[0] = this->pad05[0];
      clone->pad05[1] = this->pad05[1];
      clone->pad05[2] = this->pad05[2];
      return clone;
   }
   void enable_state_parent::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      this->ref.clear_if(my_owner, target);
   }
}