#pragma once
#include "dovah/form_stub_collection.h"
#include "./queued_ref.h"
#include "./world_cell_map.h"

namespace dovah::load_order_processes::reparent_persistent_refs {
   class process {
      public:
         void execute(form_stub_collection&);
   };
}