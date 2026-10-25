#pragma once
#include <cstdint>
#include <vector>
#include "./_base.h"
namespace dovah::tes {
   class record_view;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   class game_settings : public _base<game_settings> {
      protected:
         using _my_base = _base<game_settings>;
      public:
         using _my_base::_base;

      public:
         struct queued_group {
            uint32_t pos = 0;
         };

      public:
         std::vector<queued_group> queue;

      public:
         void execute_on_thread();

      protected:
         void _parse_record(tes::record_view&, bare_form_id form_id);
   };
}