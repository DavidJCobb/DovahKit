#pragma once
#include "../../core.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class record;
   }
   using loaded_form_factory_t = loaded_forms::Form*(*)(tes_file_reading::record&);
   loaded_form_factory_t get_loaded_form_factory_by_type(form_type_t) noexcept; // can return nullptr
}