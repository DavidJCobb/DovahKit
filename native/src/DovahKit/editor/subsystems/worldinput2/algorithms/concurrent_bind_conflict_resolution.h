#pragma once
#include "../chrono.h"

namespace dovahkit::subsystems::worldinput2 {
   class bind_list_item;
}

namespace dovahkit::subsystems::worldinput2::algorithms {
   extern void concurrent_bind_conflict_resolution(
      timestamp_t current_time,
      bind_list_item& a,
      bind_list_item& b,

      bind_list_item*& winner,
      bool& allow_activation
   );
}