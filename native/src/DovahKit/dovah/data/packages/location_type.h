#pragma once
#include <cstdint>

namespace dovah::packages {
   enum class location_type : uint8_t {
      reference                   =  0, // REFR form (warns if not persistent)
      interior_cell               =  1, // CELL form
      near_package_start_location =  2,
      near_editor_location        =  3,
      object                      =  4, // general form
      object_type                 =  5, // enum
      linked_ref                  =  6, // KYWD form
      at_package_location         =  7,
      reference_alias             =  8,
      location_alias              =  9,
      interrupt_override_target   = 10, // `interrupt_override_target` (containing PACK must have matching interrupt override type)
      // unknown 11
      self                        = 12,
   };
}