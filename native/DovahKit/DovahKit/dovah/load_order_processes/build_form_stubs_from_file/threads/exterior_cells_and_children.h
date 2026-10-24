#pragma once
#include <cstdint>
#include <vector>
#include "./_base.h"
namespace dovah {
   class form_stub;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   class exterior_cells_and_children : public _base<exterior_cells_and_children> {
      protected:
         using _my_base = _base<exterior_cells_and_children>;
      public:
         using _my_base::_base;

      public:
         // A sub-block GRUP for exterior cells.
         struct queued_group {
            form_stub* worldspace = nullptr;
            struct {
               int16_t x;
               int16_t y;
            } block;
            struct {
               int16_t x;
               int16_t y;
            } sub_block;
            uint32_t pos = 0; // file offset
         };

      public:
         std::vector<queued_group> queue;

      public:
         void execute_on_thread();
   };
}