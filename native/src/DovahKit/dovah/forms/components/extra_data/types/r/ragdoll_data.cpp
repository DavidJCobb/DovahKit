#include "./ragdoll_data.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result ragdoll_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      switch (subrecord.signature()) {
         case signature_base:
            {
               auto& v = this->data_rgd.emplace();
               v.resize(subrecord.size());
               subrecord.read(v.data(), subrecord.size());
            }
            break;
         case signature_biped:
            {
               auto& v = this->data_rgb.emplace();
               subrecord.read(v.data(), v.size());
            }
            break;
         default:
            return subrecord_load_result::unrecognized;
      }
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result ragdoll_data::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void ragdoll_data::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      if (this->data_rgd.has_value()) {
         auto& src      = this->data_rgd.value();
         auto& subrecord = record.open_next_subrecord(signature_base);
         subrecord.write(src.data(), src.size());
         subrecord.close();
      }
      if (this->data_rgb.has_value()) {
         auto& src      = this->data_rgb.value();
         auto& subrecord = record.open_next_subrecord(signature_biped);
         subrecord.write(src.data(), src.size());
         subrecord.close();
      }
   }
   
   /*static*/ void ragdoll_data::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void ragdoll_data::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void ragdoll_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* ragdoll_data::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new ragdoll_data;
      clone->data_rgd = this->data_rgd;
      clone->data_rgb = this->data_rgb;
      return clone;
   }
}