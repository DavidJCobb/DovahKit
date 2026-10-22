#pragma once
#include "lu/passkey.h"
namespace dovah {
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   class form_stub;
}

namespace dovah::_passkeys {
   using build_use_info_during_load = lu::passkey<
      form_stub,
      load_order_processes::use_info_build::use_info_builder
   >;
}