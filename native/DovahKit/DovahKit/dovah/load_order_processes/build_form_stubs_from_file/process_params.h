#pragma once
namespace dovah {
   namespace callback_sets {
      struct data_load;
   }
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   struct process_params {
      active_load_order&              active_load_order;
      const callback_sets::data_load& callbacks;
      const tes::file&                source_file;
   };
}