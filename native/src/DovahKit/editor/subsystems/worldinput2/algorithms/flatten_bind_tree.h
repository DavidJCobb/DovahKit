#pragma once
#include "../bind_list.h"

namespace dovahkit::subsystems::worldinput2::binds {
   class tree;
}

namespace dovahkit::subsystems::worldinput2::algorithms {
   extern bind_list flatten_bind_tree(
      const binds::tree& src
   );
}