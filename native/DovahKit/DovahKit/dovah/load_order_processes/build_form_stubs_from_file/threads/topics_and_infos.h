#pragma once
#include <cstdint>
#include <vector>
#include "./_base.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   //
   // NOTE: Currently, you can only have one worker thread of this type. This is 
   //       because if an ill-formed file has multiple topic-info groups which 
   //       are parented to the same topic, we would end up accessing the topic's 
   //       ordered-child list in a non-thread-safe manner.
   //
   class topics_and_infos : public _base<topics_and_infos> {
      protected:
         using _my_base = _base<topics_and_infos>;
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
   };
}