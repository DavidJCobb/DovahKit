#include "./lit_water.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result lit_water::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      auto& formID = this->refs.emplace_back();
      subrecord.read(formID);
      intfc.warn_if_ref_is_wrong_type(formID, form_type::reference, subrecord.signature());
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result lit_water::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void lit_water::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      for (auto ref : this->refs) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(ref);
         subrecord.close();
      }
   }
   
   /*static*/ void lit_water::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID))
         uib.add_outbound_reference(formID);
   }
   /*virtual*/ void lit_water::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      clear_form_reference_list(this->refs, my_owner);
   }
   /*virtual*/ void lit_water::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      remove_form_from_reference_list(this->refs, target, my_owner);
   }
   
   /*virtual*/ extra_data* lit_water::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new lit_water;
      copy_form_reference_list(clone_owner, clone->refs, this->refs);
      return clone;
   }
}