#pragma once
#include "bhkShape.h"
#include "../types/HavokMaterial.h"

namespace nifDK::block_types {
   class bhkSphereRepShape : public bhkShape {
      public:
         static constexpr const char* const type_name = "bhkSphereRepShape";
      public:
         HavokMaterial material;
         float         radius = 0;

         virtual void parse(file_reader&) override = 0; // pure but still implemented; subclasses should call super
   };
}