#pragma once
#include "../../core.h"

namespace dovah {
   class form_stub;
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class record;
   }
   using outbound_uses_builder_t = loaded_forms::Form*(*)(tes_file_reading::record&, form_stub*);
   outbound_uses_builder_t get_outbound_uses_builder_by_type(form_type_t) noexcept; // can return nullptr
}