#include "./use_info_state.h"
#include "../../../form_stub_use_info_builder.h"

namespace dovah::loaded_forms::components {
   void extra_data_use_info_state::clear() {
      for (auto& id : this->form_ids.list)
         id = 0;
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
   }
}