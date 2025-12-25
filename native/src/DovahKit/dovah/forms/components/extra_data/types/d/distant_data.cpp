#include "./distant_data.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result distant_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->unk00);
      subrecord.read(this->unk04);
      subrecord.read(this->unk08);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result distant_data::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void distant_data::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->unk00);
      subrecord.write(this->unk04);
      subrecord.write(this->unk08);
      subrecord.close();
   }
   
   /*static*/ void distant_data::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void distant_data::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void distant_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* distant_data::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new distant_data;
      clone->unk00 = this->unk00;
      clone->unk04 = this->unk04;
      clone->unk08 = this->unk08;
      return clone;
   }
}