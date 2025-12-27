#pragma once
#include "../common_form.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class ownership : public common_form<ownership, 'XOWN', (std::array{ form_type::actor_base, form_type::faction })> {
      public:
         static void generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state);
   };
}