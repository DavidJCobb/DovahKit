#include "./teleport.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result teleport::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->target_door);
      intfc.warn_if_ref_is_wrong_type(this->target_door, form_type::reference, subrecord.signature());
      subrecord.read(this->position.x);
      subrecord.read(this->position.y);
      subrecord.read(this->position.z);
      subrecord.read(this->rotation.x);
      subrecord.read(this->rotation.y);
      subrecord.read(this->rotation.z);
      subrecord.read(this->flags);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result teleport::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void teleport::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      if (!this->target_door)
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->target_door);
      subrecord.write(this->position.x);
      subrecord.write(this->position.y);
      subrecord.write(this->position.z);
      subrecord.write(this->rotation.x);
      subrecord.write(this->rotation.y);
      subrecord.write(this->rotation.z);
      subrecord.write(this->flags);
      subrecord.close();
   }
   
   /*static*/ void teleport::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      subrecord.read(uis.form_ids_with_flags.teleport.target_door);
   }
   /*virtual*/ void teleport::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->target_door.set(my_owner, nullptr);
   }
   /*virtual*/ void teleport::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->target_door.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* teleport::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new teleport;
      clone->target_door.set(clone_owner, this->target_door);
      clone->position = this->position;
      clone->rotation = this->rotation;
      clone->flags = this->flags;
      return clone;
   }
}