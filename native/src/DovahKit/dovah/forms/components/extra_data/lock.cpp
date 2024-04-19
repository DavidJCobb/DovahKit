#include "lock.h"
#include "../../_common_cpp.h"
#include "_use_info.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result lock::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->level);
      subrecord.skip_bytes(3);
      subrecord.read(this->key);
      intfc.warn_if_ref_is_wrong_type(this->key, form_type::key, subrecord.signature());
      subrecord.read(this->flags);
      subrecord.skip_bytes(3);
      subrecord.read(this->unk0C);
      subrecord.read(this->unk10);
      return load_result::succeeded;
   }
   void lock::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->level);
      subrecord.skip_bytes(3);
      subrecord.write(this->key);
      subrecord.write(this->flags);
      subrecord.skip_bytes(3);
      subrecord.write(this->unk0C);
      subrecord.write(this->unk10);
      subrecord.close();
   }
   //
   /*static*/ void lock::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      auto& subrecord = record.get_current_subrecord();
      subrecord.skip_bytes(4);
      subrecord.read(state.by_name.lock.key);
   }
   basic_extra_data* lock::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new lock;
      clone->level = this->level;
      clone->key.set(clone_owner, this->key);
      clone->flags = this->flags;
      clone->unk0C = this->unk0C;
      clone->unk10 = this->unk10;
      return clone;
   }
   void lock::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      this->key.clear_if(my_owner, target);
   }
}