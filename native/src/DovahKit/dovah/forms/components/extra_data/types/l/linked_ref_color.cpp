#include "./linked_ref_color.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result linked_ref_color::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      this->start.load(subrecord);
      this->end.load(subrecord);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result linked_ref_color::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void linked_ref_color::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      this->start.save(subrecord);
      this->end.save(subrecord);
      subrecord.close();
   }
   
   /*static*/ void linked_ref_color::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void linked_ref_color::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void linked_ref_color::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* linked_ref_color::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new linked_ref_color;
      clone->start = this->start;
      clone->end   = this->end;
      return clone;
   }
}