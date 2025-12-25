#include "./linked_ref.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result linked_ref::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      if (subrecord.size() >= 8) {
         subrecord.read(this->keyword);
         intfc.warn_if_ref_is_wrong_type(this->keyword, form_type::keyword, subrecord.signature());
      }
      subrecord.read(this->ref);
      intfc.warn_if_ref_is_wrong_type(this->ref, form_type::reference, subrecord.signature());
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result linked_ref::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void linked_ref::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      if (!this->ref)
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      if (this->keyword)
         subrecord.write(this->keyword);
      subrecord.write(this->ref);
      subrecord.close();
   }
   
   /*static*/ void linked_ref::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.is_in_bounds(8)) {
         subrecord.read(uis.form_ids.by_name.linked_ref.keyword);
         subrecord.read(uis.form_ids.by_name.linked_ref.ref);
         return;
      }
      subrecord.read(uis.form_ids.by_name.linked_ref.ref);
   }
   /*virtual*/ void linked_ref::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->keyword.set(my_owner, nullptr);
      this->ref.set(my_owner, nullptr);
   }
   /*virtual*/ void linked_ref::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->keyword.clear_if(my_owner, target);
      this->ref.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* linked_ref::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new linked_ref;
      clone->keyword.set(clone_owner, this->keyword);
      clone->ref.set(clone_owner, this->ref);
      return clone;
   }
}