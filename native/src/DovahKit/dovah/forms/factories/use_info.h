#pragma once
#include "../../form_types.h"

namespace dovah {
   class form_stub;
   class form_stub_use_info_builder;
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class record;
   }
   using outbound_uses_builder_t = void(*)(tes_file_reading::record&, form_stub_use_info_builder&);
   outbound_uses_builder_t get_outbound_uses_builder_by_type(form_type) noexcept; // can return nullptr
}