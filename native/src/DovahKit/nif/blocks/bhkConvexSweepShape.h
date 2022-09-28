#pragma once
#include <glm/glm.hpp>
#include "bhkShape.h"
#include "../types/HavokMaterial.h"

namespace nifDK::block_types {
   class bhkConvexSweepShape : public bhkShape {
      public:
         static constexpr const char* const type_name = "bhkConvexSweepShape";
      public:
         bhkShape*     subject = nullptr;
         HavokMaterial material;
         float         radius = 0;
         glm::fvec3    unknown;

         virtual void parse(file_reader&) override;
   };
}