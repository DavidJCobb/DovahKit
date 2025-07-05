#pragma once
#include <cstdint>

namespace dovah::packages {
   enum class target_type : uint32_t {
      reference                 = 0, // REFR form (warns if not persistent)
      object                    = 1, // general form
      object_type               = 2,
      linked_ref                = 3, // KYWD form
      reference_alias           = 4,
      interrupt_override_target = 5, // `interrupt_override_target` (containing PACK must have matching interrupt override type)
      self                      = 6,
   };
}