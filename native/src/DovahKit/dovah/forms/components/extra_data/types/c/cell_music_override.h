#pragma once
#include "../common_form.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class cell_music_override : public common_form<cell_music_override, 'XCMO', form_type::music_type> {
      public:
         static void generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state);
   };
}