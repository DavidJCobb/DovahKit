#pragma once
#include <cstdint>
#include "../blocks/bhkEntity.h"
#include "./HavokConstraintDescriptorVariant.h"
#include "./hkConstraintType.h"

namespace nifDK {
   class file_reader;

   struct ConstraintData {
      block_types::bhkEntity* target_a = nullptr;
      block_types::bhkEntity* target_b = nullptr;
      uint32_t priority = 1;
      HavokConstraintDescriptorVariant data;

      void read(file_reader&);
   };
}