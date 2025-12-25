#include "./navmesh_door_portal.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result navmesh_door_portal::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->navmesh);
      intfc.warn_if_ref_is_wrong_type(this->navmesh, form_type::navmesh, subrecord.signature());
      subrecord.read(this->triangle);
      subrecord.read(this->pad06);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result navmesh_door_portal::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void navmesh_door_portal::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->navmesh);
      subrecord.write(this->triangle);
      subrecord.write(this->pad06);
      subrecord.close();
   }
   
   /*static*/ void navmesh_door_portal::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      subrecord.read(uis.form_ids.by_name.navmesh_door_portal.navmesh);
   }
   /*virtual*/ void navmesh_door_portal::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->navmesh.set(my_owner, nullptr);
   }
   /*virtual*/ void navmesh_door_portal::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->navmesh.clear_if(my_owner, target);
   }
   
   /*virtual*/ extra_data* navmesh_door_portal::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new navmesh_door_portal;
      clone->navmesh.set(clone_owner, this->navmesh);
      clone->triangle = this->triangle;
      clone->pad06 = this->pad06;
      return clone;
   }
}