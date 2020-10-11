#include "lock.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result lock::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->level);
      subrecord.read(this->pad01);
      subrecord.read(this->key);
      subrecord.read(this->flags);
      subrecord.read(this->pad09);
      subrecord.read(this->unk0C);
      subrecord.read(this->unk10);
      return load_result::succeeded;
   }
   void lock::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->level);
      subrecord.write(this->pad01);
      subrecord.write(this->key);
      subrecord.write(this->flags);
      subrecord.write(this->pad09);
      subrecord.write(this->unk0C);
      subrecord.write(this->unk10);
      subrecord.close();
   }
   //
   /*static*/ void lock::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      subrecord.skip_bytes(sizeof(level) + sizeof(pad01));
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
   }
   basic_extra_data* lock::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new lock;
      clone->level = this->level;
      clone->pad01[0] = this->pad01[0];
      clone->pad01[1] = this->pad01[1];
      clone->pad01[2] = this->pad01[2];
      clone->key.set(clone_owner, this->key);
      clone->flags = this->flags;
      clone->pad09[0] = this->pad09[0];
      clone->pad09[1] = this->pad09[1];
      clone->pad09[2] = this->pad09[2];
      clone->unk0C = this->unk0C;
      clone->unk10 = this->unk10;
      return clone;
   }
   void lock::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      this->key.clear_if(my_owner, target);
   }
}