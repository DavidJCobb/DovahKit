#pragma once
#include "NiTriBasedGeomData.h"
#include "../types/Triangle.h"

namespace nifDK::block_types {
   class NiTriShapeData : public NiTriBasedGeomData {
      public:
         static constexpr const char* const type_name = "NiTriShapeData";
      public:
         struct match_group {
            std::vector<uint16_t> vertex_indices;
         };

         std::vector<Triangle>    triangles;
         std::vector<match_group> match_groups;

         virtual void parse(file_reader&) override;
   };
}