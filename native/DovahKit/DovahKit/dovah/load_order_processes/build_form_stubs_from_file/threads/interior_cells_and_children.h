#pragma once
#include <cstdint>
#include <vector>
#include "./_base.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   class interior_cells_and_children : public _base<interior_cells_and_children> {
      protected:
         using _my_base = _base<interior_cells_and_children>;
      public:
         using _my_base::_base;

      public:
         // A sub-block GRUP for interior cells.
         struct queued_group {
            uint32_t block_number = 0;
            uint32_t pos          = 0;
         };

      public:
         std::vector<queued_group> queue;

      public:
         void execute_on_thread();
   };
}