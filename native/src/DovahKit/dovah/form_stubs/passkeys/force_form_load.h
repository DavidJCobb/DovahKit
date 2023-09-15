#pragma once

namespace dovah {
   namespace tes_file_reading {
      class load_order_persistent_ref_reparenter;
   }
   namespace tes_file_writing {
      class file_writer;
   }
   class refs_need_persistence_checker;
}

namespace dovah::form_stub_passkeys {
   class force_form_load {
      friend dovah::tes_file_reading::load_order_persistent_ref_reparenter;
      friend dovah::tes_file_writing::file_writer;
      friend dovah::refs_need_persistence_checker;
      private:
         constexpr force_form_load() {}
   };
}