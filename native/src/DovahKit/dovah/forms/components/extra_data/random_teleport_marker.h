#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class random_teleport_marker : public formID_extra_data<'XRTM', extra_data_type::random_teleport_marker> {
      // The form should be a REFR.
      public:
         static void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.random_teleport_marker);
         }
   };
}