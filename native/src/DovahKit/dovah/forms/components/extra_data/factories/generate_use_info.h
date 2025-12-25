#pragma once
namespace dovah {
   namespace loaded_forms::components {
      class extra_data_use_info_state;
   }
   namespace tes_file_reading {
      class record;
   }
   class form_stub_use_info_builder;
}

namespace dovah::loaded_forms::components::extra_data_factories {
   // Returns false if the current subrecord matches no extra data.
   extern bool generate_use_info(tes_file_reading::record&, form_stub_use_info_builder&, extra_data_use_info_state&);
}