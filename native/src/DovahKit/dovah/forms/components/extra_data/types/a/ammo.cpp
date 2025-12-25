#include "./ammo.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result ammo::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      switch (subrecord.signature()) {
         case signature_type:
            subrecord.read(this->type);
            intfc.warn_if_ref_is_wrong_type(this->type, form_type::ammo, subrecord.signature());
            this->count = 999; // executable-level default
            break;
         case signature_count:
            subrecord.read(this->count);
            break;
         default:
            return subrecord_load_result::unrecognized;
      }
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result ammo::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void ammo::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      if (!this->type)
         return;
      record.write_formID_subrecord(signature_type, this->type);
      auto& XPSC = record.open_next_subrecord(signature_count);
      XPSC.write(this->count);
      XPSC.close();
   }
   
   /*static*/ void ammo::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_type) {
         subrecord.read(uis.form_ids.by_name.ammo.type);
      }
   }
   /*virtual*/ void ammo::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->type.set(my_owner, nullptr);
   }
   /*virtual*/ void ammo::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->type.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* ammo::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new ammo;
      clone->type.set(clone_owner, this->type);
      clone->count = this->count;
      return clone;
   }
}