#pragma once

namespace dovah {
   namespace tes_file_reading {
      class file_or_file_part_loader;
      class threaded_load_order_use_info_builder;
      class load_order_persistent_ref_reparenter;
   }
   class file_load_order;
   class form_stub;
   class form_stub_use_info_builder;
}

namespace dovah::form_stub_passkeys {
   class build_use_info_during_load {
      friend dovah::tes_file_reading::file_or_file_part_loader;
      friend dovah::tes_file_reading::threaded_load_order_use_info_builder;
      friend dovah::tes_file_reading::load_order_persistent_ref_reparenter;
      friend dovah::file_load_order;
      friend dovah::form_stub;
      friend dovah::form_stub_use_info_builder;
      private:
         constexpr build_use_info_during_load() {}
   };
}