#include "./enable_state_parent.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result enable_state_parent::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->ref);
      intfc.warn_if_ref_is_wrong_type(this->ref, form_type::reference, subrecord.signature());
      subrecord.read(this->flags);
      subrecord.read(this->pad05);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result enable_state_parent::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void enable_state_parent::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->ref);
      subrecord.write(this->flags);
      subrecord.write(this->pad05);
      subrecord.close();
   }
   
   /*static*/ void enable_state_parent::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature) {
         subrecord.read(uis.form_ids.by_name.enable_state_parent.ref);
      }
   }
   /*virtual*/ void enable_state_parent::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->ref.set(my_owner, nullptr);
   }
   /*virtual*/ void enable_state_parent::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->ref.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* enable_state_parent::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new enable_state_parent;
      clone->ref.set(clone_owner, this->ref);
      clone->flags = this->flags;
      clone->pad05[0] = this->pad05[0];
      clone->pad05[1] = this->pad05[1];
      clone->pad05[2] = this->pad05[2];
      return clone;
   }
}