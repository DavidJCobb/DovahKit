#pragma once
#include "./string_to_id_maps.h"
namespace dovah {
   namespace load_order_processes::active_file_strings_to_localized {
      class process_pimpl;
   }
   class base_managed_form_data;
   class form_stub;
}

namespace dovah::load_order_processes::active_file_strings_to_localized {
   extern void convert_strings_in_form(
      process_pimpl&,
      string_to_id_maps& strings_to_ids,
      form_stub&,
      base_managed_form_data&
   );
}