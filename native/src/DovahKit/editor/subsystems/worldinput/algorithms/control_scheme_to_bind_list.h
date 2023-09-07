#pragma once
#include "../bind_list.h"

namespace dovahkit::subsystems::worldinput {
   class control_scheme;
}

namespace dovahkit::subsystems::worldinput::algorithms {
   extern bind_list control_scheme_to_bind_list(
      const control_scheme& src
   );
}