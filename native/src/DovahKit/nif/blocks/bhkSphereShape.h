#pragma once
#include "bhkConvexShape.h"

namespace nifDK::block_types {
   class bhkSphereShape : public bhkConvexShape {
      public:
         static constexpr const char* const type_name = "bhkSphereShape";
      public:
         virtual void parse(file_reader& reader) override {
            bhkConvexShape::parse(reader);
         }
   };
}