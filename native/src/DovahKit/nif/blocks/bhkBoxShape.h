#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "bhkConvexShape.h"

namespace nifDK::block_types {
   class bhkBoxShape : public bhkConvexShape {
      public:
         static constexpr const char* const type_name = "bhkBoxShape";
      public:

         uint64_t   pad00;
         glm::fvec4 dimensions; // w-component is unused

         virtual void parse(file_reader&) override;
   };
}