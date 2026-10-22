#pragma once
#include "dovah/types/cells/grid_position.h"
namespace dovah {
   class form_stub;
}

namespace dovah::load_order_processes::reparent_persistent_refs {
   struct queued_ref {
      form_stub* ref  = nullptr;
      form_stub* cell = nullptr;
      types::cells::grid_position grid_position;
   };
}