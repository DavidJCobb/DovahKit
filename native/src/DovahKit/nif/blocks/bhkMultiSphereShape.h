#pragma once
#include "bhkSphereRepShape.h"
#include "../types/NiBound.h"

namespace nifDK::block_types {
   class bhkMultiSphereShape : public bhkSphereRepShape {
      public:
         static constexpr const char* const type_name = "bhkMultiSphereShape";
      public:
         float unknown_1;
         float unknown_2;
         std::vector<NiBound> spheres;

         virtual void parse(file_reader& reader) override;
   };
}