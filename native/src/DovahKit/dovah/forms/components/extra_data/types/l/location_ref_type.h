#pragma once
#include "../common_unique_form.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class location_ref_type : public common_unique_form<location_ref_type, 'XLRT', form_type::location_ref_type, use_info::entry_flags::base_extra_data::extra_location_ref_type> {
      public:
         static void generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state);
   };
}