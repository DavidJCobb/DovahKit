#pragma once
#include <array>
#include "NiObject.h"
#include "../types/NiBound.h"
#include "../types/NiColor.h"

namespace nifDK::block_types {
   class NiGeometryData : public NiObject {
      public:
         static constexpr const char* const type_name = "NiGeometryData";
      public:
         using uv_set = std::vector<glm::fvec2>; // OpenGL standard

         struct vector_flag {
            enum type : uint16_t {
               has_uv       = 1 <<  0,
               has_tangents = 1 << 12,
            };
         };
         using vector_flags = std::underlying_type_t<vector_flag::type>;

         int32_t group_id = 0; // always 0
         struct {
            uint8_t keep     = 0;
            uint8_t compress = 0;
            vector_flags vector = 0;
         } flags;
         std::vector<glm::fvec3> vertices;
         std::vector<NiColorA> vertex_colors;
         std::vector<uv_set> uv_sets;
         std::vector<glm::fvec3> normals;
         std::vector<glm::fvec3> tangents;
         std::vector<glm::fvec3> bitangents;
         std::vector<float> unknown_floats;
         uint32_t material_crc = 0;
         NiBound bounds;
         std::array<uint16_t, 13> unknown_shorts = {};
         uint16_t consistency_flags = 0;
         NiObject* additional = nullptr;

         virtual void parse(file_reader&) override;
   };
}