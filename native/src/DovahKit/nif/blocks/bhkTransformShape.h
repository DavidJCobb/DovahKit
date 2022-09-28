#pragma once
#include <glm/glm.hpp>
#include "bhkShape.h"
#include "../types/HavokMaterial.h"

namespace nifDK::block_types {
   class bhkTransformShape : public bhkShape {
      public:
         static constexpr const char* const type_name = "bhkTransformShape";
      public:
         bhkShape*     subject = nullptr;
         HavokMaterial material;
         float         radius = 0;
         uint64_t      pad;
         glm::fmat4x4  transform = glm::fmat4x4(1); // TODO: Column-major or row-major?

         virtual void parse(file_reader&) override;
   };
}