#pragma once
#include <glm/glm.hpp>
#include "bhkShapePhantom.h"

namespace nifDK::block_types {
   class bhkSimpleShapePhantom : public bhkShapePhantom {
      public:
         static constexpr const char* const type_name = "bhkSimpleShapePhantom";
      public:
         uint64_t pad00;
         glm::fmat4x4 transform;

         virtual void parse(file_reader&) override;
   };
}