#include "./patrol_ref_data.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result patrol_ref_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      switch (subrecord.signature()) {
         case signature_time:
            subrecord.read(this->idle_time);
            break;
         case signature_event:
            return subrecord_load_result::requires_record;
         default:
            return subrecord_load_result::unrecognized;
      }
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result patrol_ref_data::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      if (this->event.load(record, intfc))
         return record_load_result::complete;
      return record_load_result::incomplete;
   }
   /*virtual*/ void patrol_ref_data::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& XPRD = record.open_next_subrecord(signature_time);
      XPRD.write(this->idle_time);
      XPRD.close();
      record.open_next_subrecord(signature_event).close(); // empty; acts as a header for `this->event`
      this->event.save(record);
   }
   
   /*static*/ void patrol_ref_data::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_event) {
         uis.patrol_ref_data.generate_use_info(record);
      }
   }
   /*virtual*/ void patrol_ref_data::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->event.clear(my_owner);
   }
   /*virtual*/ void patrol_ref_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->event.sever_outbound_references_to(target, my_owner);
   }
   
   /*virtual*/ extra_data* patrol_ref_data::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new patrol_ref_data;
      clone->idle_time = this->idle_time;
      clone->event.clone_from(this->event, clone_owner);
      return clone;
   }
}