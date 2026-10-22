#pragma once
#include <cstdint>
#include "lu/integral_character_code.h"
#include "./_base.h"
namespace dovah {
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   class forms_of_type : public _base<forms_of_type> {
      public:
         struct queued_group {
            lu::four_cc signature = 0;
            uint32_t    pos       = 0;
         };

      public:
         active_load_order* active_load_order = nullptr;
         const tes::file*   source_file       = nullptr;
         std::vector<queued_group> queue;

      public:
         void execute_on_thread();
   };
}