#pragma once
#include "../common_unique_form.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class encounter_zone : public common_unique_form<encounter_zone, 'XEZN', form_type::encounter_zone, use_info::entry_flags::base_extra_data::extra_encounter_zone> {
      public:
         static void generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state);
   };
}