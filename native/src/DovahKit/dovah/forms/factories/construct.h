#pragma once
#include "../../core.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class record;
   }
   using loaded_form_load_function_t = loaded_forms::Form*(*)(tes_file_reading::record&); // construct and load
   extern loaded_form_load_function_t get_loaded_form_factory_by_type(form_type_t) noexcept; // can return nullptr

   extern loaded_forms::Form* create_blank_loaded_form_by_type(form_type_t) noexcept;
}