#include "teleport.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result teleport::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->target_door);
      subrecord.read(this->position.x);
      subrecord.read(this->position.y);
      subrecord.read(this->position.z);
      subrecord.read(this->rotation.x);
      subrecord.read(this->rotation.y);
      subrecord.read(this->rotation.z);
      subrecord.read(this->flags);
      return load_result::succeeded;
   }
   void teleport::save(tes_record_writer& record) {
      if (!this->target_door)
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->target_door);
      subrecord.write(this->position.x);
      subrecord.write(this->position.y);
      subrecord.write(this->position.z);
      subrecord.write(this->rotation.x);
      subrecord.write(this->rotation.y);
      subrecord.write(this->rotation.z);
      subrecord.write(this->flags);
      subrecord.close();
   }
   //
   /*static*/ void teleport::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         uib.add_outbound_reference(formID);
   }
   basic_extra_data* teleport::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new teleport;
      clone->target_door.set(clone_owner, this->target_door);
      clone->position = this->position;
      clone->rotation = this->rotation;
      clone->flags    = this->flags;
      return clone;
   }
   void teleport::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      this->target_door.clear_if(my_owner, target);
   }
}