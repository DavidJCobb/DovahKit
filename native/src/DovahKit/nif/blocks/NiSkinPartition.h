#pragma once
#include <cstdint>
#include <vector>
#include "NiObject.h"
#include "../types/BSVertexDataSSE.h"
#include "../types/BSVertexDesc.h"
#include "../types/Triangle.h"

namespace nifDK::block_types {
   class NiSkinPartition : public NiObject {
      public:
         static constexpr const char* const type_name = "NiSkinPartition";
      public:
         struct partition {
            using strip = std::vector<uint16_t>;

            uint16_t weights_per_vertex = 0;
            std::vector<uint16_t> bones;
            std::vector<uint16_t> vertex_map;
            std::vector<float>    weights;
            std::vector<strip>    strips;    // mutually exclusive with triangles
            std::vector<Triangle> triangles; // mutually exclusive with strips; strips have priority
            std::vector<uint8_t>  bone_indices;
            uint16_t unknown;
            struct {
               BSVertexDesc vertex_description;
               std::vector<Triangle> triangles;
            } remaster;

            void parse(file_reader&);
         };

      public:
         std::vector<partition> partitions;
         struct {
            BSVertexDesc vertex_description;
            std::vector<BSVertexDataSSE> vertices;
         } remaster;

         virtual void parse(file_reader&) override;
   };
}