#include "./multibound_bounds.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result multibound_bounds::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->halfwidths.x);
      subrecord.read(this->halfwidths.y);
      subrecord.read(this->halfwidths.z);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result multibound_bounds::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void multibound_bounds::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->halfwidths.x);
      subrecord.write(this->halfwidths.y);
      subrecord.write(this->halfwidths.z);
      subrecord.close();
   }
   
   /*static*/ void multibound_bounds::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void multibound_bounds::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void multibound_bounds::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* multibound_bounds::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new multibound_bounds;
      clone->halfwidths = this->halfwidths;
      return clone;
   }
}