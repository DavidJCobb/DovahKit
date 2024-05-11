#pragma once
#include <cstdint>

namespace dovah::conditions {
   enum class run_on_type : uint32_t {
      subject       = 0,
      target        = 1,
      reference     = 2, // form reference
      combat_target = 3,
      linked_ref    = 4,
      quest_alias   = 5, // ID of an alias on this condition's owning quest
      package_data  = 6, // index of a package-data on this condition's owning package
      event_data    = 7, // event data index
   };
}