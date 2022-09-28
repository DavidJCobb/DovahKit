#pragma once
#include <cstdint>
#include "NiTriBasedGeomData.h"

namespace nifDK::block_types {
   class NiTriStripsData : public NiTriBasedGeomData {
      public:
         static constexpr const char* const type_name = "NiTriStripsData";
      public:
         std::vector<uint16_t> strip_lengths; // number of points in each strip
         std::vector<uint16_t> points;

         virtual void parse(file_reader&) override;
   };
}