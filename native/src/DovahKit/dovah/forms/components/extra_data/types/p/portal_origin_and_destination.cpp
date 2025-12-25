#include "./portal_origin_and_destination.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result portal_origin_and_destination::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->origin);
      subrecord.read(this->destination);
      intfc.warn_if_ref_is_wrong_type(this->origin, form_type::reference, subrecord.signature());
      intfc.warn_if_ref_is_wrong_type(this->destination, form_type::reference, subrecord.signature());
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result portal_origin_and_destination::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void portal_origin_and_destination::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->origin);
      subrecord.write(this->destination);
      subrecord.close();
   }
   
   /*static*/ void portal_origin_and_destination::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      subrecord.read(uis.form_ids.by_name.portal_origin_and_destination.origin);
      subrecord.read(uis.form_ids.by_name.portal_origin_and_destination.destination);
   }
   /*virtual*/ void portal_origin_and_destination::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->origin.set(my_owner, nullptr);
      this->destination.set(my_owner, nullptr);
   }
   /*virtual*/ void portal_origin_and_destination::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->origin.clear_if(my_owner, target);
      this->destination.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* portal_origin_and_destination::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new portal_origin_and_destination;
      clone->origin.set(clone_owner, this->origin);
      clone->destination.set(clone_owner, this->destination);
      return clone;
   }
}