#pragma once

namespace dovah {
   namespace tes_file_reading {
      class load_order_persistent_ref_reparenter;
   }
}

namespace dovah::form_stub_passkeys {
   class do_custom_parse_during_load {
      friend dovah::tes_file_reading::load_order_persistent_ref_reparenter;
      private:
         constexpr do_custom_parse_during_load() {}
   };
}