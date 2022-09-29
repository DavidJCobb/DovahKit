#pragma once
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include "bhkConvexShape.h"

namespace nifDK::block_types {
   class bhkCapsuleShape : public bhkConvexShape {
      public:
         static constexpr const char* const type_name = "bhkCapsuleShape";
      public:
         struct endcap {
            glm::fvec3 pos;
            float      radius;
         };

         uint64_t pad00;
         std::array<endcap, 2> endcaps;

         virtual void parse(file_reader&) override;
   };
}