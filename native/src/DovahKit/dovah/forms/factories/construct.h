#pragma once
#include "../../core.h"
#include "../Form.h"

namespace dovah {
   namespace tes_file_reading {
      class record;
   }
   namespace load_order_interfaces {
      class form_load;
   }
   using form_loader_function_t = void(*)(loaded_forms::Form*, tes_file_reading::record&, load_order_interfaces::form_load&); // construct and load

   extern form_loader_function_t get_form_loader_function(form_type) noexcept;
   extern loaded_forms::Form* create_blank_loaded_form_by_type(form_type, const loaded_forms::Form::constructor_params&) noexcept;

   extern bool can_construct_form_data(form_type) noexcept;
   extern bool can_load_form_data(form_type) noexcept;
}