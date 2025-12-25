#include "./lock.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result lock::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->level);
      subrecord.skip_bytes(3);
      subrecord.read(this->key);
      intfc.warn_if_ref_is_wrong_type(this->key, form_type::key, subrecord.signature());
      subrecord.read(this->flags);
      subrecord.skip_bytes(3);
      subrecord.read(this->unk0C);
      subrecord.read(this->unk10);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result lock::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void lock::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
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
   
   /*static*/ void lock::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      subrecord.skip_bytes(4);
      subrecord.read(uis.form_ids.by_name.lock.key);
   }
   /*virtual*/ void lock::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->key.set(my_owner, nullptr);
   }
   /*virtual*/ void lock::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->key.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* lock::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new lock;
      clone->level = this->level;
      clone->key.set(clone_owner, this->key);
      clone->flags = this->flags;
      clone->unk0C = this->unk0C;
      clone->unk10 = this->unk10;
      return clone;
   }
}