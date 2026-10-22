#pragma once
namespace dovah {
   class active_load_order;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   class process {
      public:
         void execute(active_load_order&);
   };
}