#pragma once
#include "../common_form.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class emittance_source : public common_form<emittance_source, 'XEMI', std::array{ form_type::light, form_type::region }> {
      public:
         static void generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state);
   };
}