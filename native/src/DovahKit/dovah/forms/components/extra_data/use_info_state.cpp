#include "./use_info_state.h"
#include "../../../form_stub_use_info_builder.h"
#include "../../../use_info/entry_flags/base_extra_data.h"

namespace dovah::loaded_forms::components {
   void extra_data_use_info_state::clear() {
      for (auto& id : this->form_ids.list)
         id = 0;
      this->form_ids_with_flags = {};
      this->cell_region_list.clear();
      this->patrol_ref_data.clear();
      this->water_current_zone_data.refs.clear();
   }
   void extra_data_use_info_state::commit_to(form_stub_use_info_builder& uib) {
      for (auto id : this->form_ids.list)
         if (id)
            uib.add_outbound_reference(id);
      for(auto id : this->cell_region_list)
         if (id)
            uib.add_outbound_reference(id);
      this->patrol_ref_data.commit_to(uib);
      for (auto id : this->water_current_zone_data.refs)
         if (id)
            uib.add_outbound_reference(id);

      {
         auto id = this->form_ids_with_flags.encounter_zone;
         if (id)
            uib.add_outbound_reference(id, use_info::entry_flags::base_extra_data::extra_encounter_zone);
      }
      {
         auto id = this->form_ids_with_flags.location;
         if (id)
            uib.add_outbound_reference(id, use_info::entry_flags::base_extra_data::extra_location);
      }
      {
         auto id = this->form_ids_with_flags.location_ref_type;
         if (id)
            uib.add_outbound_reference(id, use_info::entry_flags::base_extra_data::extra_location_ref_type);
      }
      {
         auto id = this->form_ids_with_flags.teleport.target_door;
         if (id)
            uib.add_outbound_reference(id, use_info::entry_flags::base_extra_data::extra_teleport_destination);
      }
   }
}