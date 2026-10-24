#pragma once
#include <cstdint>
#include <vector>
#include "./_base.h"
namespace dovah {
   class form_stub;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   class persistent_cell_children : public _base<persistent_cell_children> {
      protected:
         using _my_base = _base<persistent_cell_children>;
      public:
         using _my_base::_base;

      public:
         // The "Cell Children" group for a persistent cell.
         struct queued_group {
            form_stub* cell = nullptr;
            uint32_t   pos  = 0;
         };

      public:
         std::vector<queued_group> queue;

      public:
         void execute_on_thread();
   };
}