#pragma once
#include "lu/passkey.h"
namespace dovah {
   namespace load_order_processes::reparent_persistent_refs {
      class make_parenthood_bidirectional_worker;
      class set_parent_cell_worker;
   }
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   class active_load_order;
   class form_stub;
}

namespace dovah::_passkeys {
   using build_use_info_during_load = lu::passkey<
      active_load_order,
      form_stub,
      load_order_processes::reparent_persistent_refs::make_parenthood_bidirectional_worker,
      load_order_processes::reparent_persistent_refs::set_parent_cell_worker,
      load_order_processes::use_info_build::use_info_builder
   >;
}