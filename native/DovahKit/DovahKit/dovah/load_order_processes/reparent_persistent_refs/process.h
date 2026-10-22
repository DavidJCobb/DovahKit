#pragma once
#include "./queued_ref.h"
#include "./world_cell_map.h"
namespace dovah {
   class form_stub_collection;
}

namespace dovah::load_order_processes::reparent_persistent_refs {
   class process {
      public:
         void execute(form_stub_collection&);
   };
}