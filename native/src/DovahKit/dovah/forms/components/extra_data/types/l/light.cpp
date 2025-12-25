#include "./light.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result light::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->fov);
      subrecord.read(this->fade);
      subrecord.read(this->end_distance_cap);
      subrecord.read(this->shadow_depth_bias);
      subrecord.read(this->unk10); // optional
      subrecord.read(this->unk11); // optional
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result light::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void light::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->fov);
      subrecord.write(this->fade);
      subrecord.write(this->end_distance_cap);
      subrecord.write(this->shadow_depth_bias);
      subrecord.write(this->unk10);
      subrecord.write(this->unk11);
      subrecord.close();
   }
   
   /*static*/ void light::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void light::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void light::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* light::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new light;
      clone->fov = this->fov;
      clone->fade = this->fade;
      clone->end_distance_cap = this->end_distance_cap;
      clone->shadow_depth_bias = this->shadow_depth_bias;
      clone->unk10 = this->unk10;
      clone->unk11[0] = this->unk11[0];
      clone->unk11[1] = this->unk11[1];
      clone->unk11[2] = this->unk11[2];
      return clone;
   }
}