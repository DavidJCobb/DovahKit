#pragma once
#include <cstdint>
#include "dovah/data/body_parts.h"
#include "dovah/data/collision_layers.h"

namespace nifDK {
   class file_reader;

   struct HavokFilter {
      struct {
         bool linked = false;
         bool disable_collision = false;
         bool scaled = false;
      } flags;
      dovah::body_part       body_part       = dovah::body_part::other;
      dovah::collision_layer collision_layer = dovah::collision_layer::null;
      uint16_t group = 0;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}