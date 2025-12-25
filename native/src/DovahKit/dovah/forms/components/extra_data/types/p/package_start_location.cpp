#include "./package_start_location.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result package_start_location::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->unk00);
      subrecord.read(this->unk04);
      subrecord.read(this->unk08);
      subrecord.read(this->unk0C);
      subrecord.read(this->unk10);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result package_start_location::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void package_start_location::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->unk00);
      subrecord.write(this->unk04);
      subrecord.write(this->unk08);
      subrecord.write(this->unk0C);
      subrecord.write(this->unk10);
      subrecord.close();
   }
   
   /*static*/ void package_start_location::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void package_start_location::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void package_start_location::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* package_start_location::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new package_start_location;
      clone->unk00 = this->unk00;
      clone->unk04 = this->unk04;
      clone->unk08 = this->unk08;
      clone->unk0C = this->unk0C;
      clone->unk10 = this->unk10;
      return clone;
   }
}