#pragma once
#include <array>
#include "NiTriBasedGeom.h"

namespace nifDK::block_types {
   class BSLODTriShape : public NiTriBasedGeom {
      public:
         static constexpr const char* const type_name = "BSLODTriShape";
      public:
         std::array<uint32_t, 3> lod_sizes;

         virtual void parse(file_reader&) override;
   };
}