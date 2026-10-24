#pragma once
#include <cstdint>
#include "lu/integral_character_code.h"
#include "./_base.h"
namespace dovah {
   namespace callback_sets {
      struct data_load;
   }
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   class forms_of_type : public _base<forms_of_type> {
      protected:
         using _my_base = _base<forms_of_type>;
      public:
         using _my_base::_base;

      public:
         struct queued_group {
            lu::four_cc signature = 0;
            uint32_t    pos       = 0;
         };

      public:
         std::vector<queued_group> queue;

      public:
         void execute_on_thread();
   };
}