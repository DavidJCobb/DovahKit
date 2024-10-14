#pragma once
#include <array>
#include <optional>
#include "NiObject.h"
#include "NiGeometryData.h"

namespace nifDK::block_types {
   class NiParticlesData : public NiObject {
      public:
         static constexpr const char* const type_name = "NiParticlesData";
      public:
         //
         // Prior to Bethesda 20.2.0.7, NiParticlesData subclassed NiGeometryData. In 
         // 20.2.0.7, it subclasses NiObject directly. Additionally, for NiPSysData 
         // specifically, there's tons of stuff we never load.
         //
         using uv_set       = NiGeometryData::uv_set;
         using vector_flag  = NiGeometryData::vector_flag;
         using vector_flags = NiGeometryData::vector_flags;

         struct geometry_data {
            public:
               bool uses_abbreviated_vertices = false;
               //
               std::vector<glm::fvec3>  vertices;
               std::vector<NiColorA>    vertex_colors;
               std::vector<uv_set>      uv_sets;
               std::vector<glm::fvec3>  normals;
               std::vector<glm::fvec3>  tangents;
               std::vector<glm::fvec3>  bitangents;
               std::vector<float>       unknown_floats;
               std::array<uint16_t, 13> unknown_shorts = {};

            protected:
               static bool can_use_abbreviated_vertices(const file_reader&);
               static bool has_unknown_values(const file_reader&);

            public:
               void parse_vertices(file_reader&, size_t vertex_count);
               static void skip_vertices(file_reader&, size_t vertex_count, bool& uses_abbreviated_vertices);

               void parse_normals(file_reader&, size_t vertex_count, bool has_tangents);
               static void skip_normals(file_reader&, size_t vertex_count, bool has_tangents);

               void parse_unknown_floats(file_reader&, size_t vertex_count);
               static void skip_unknown_floats(file_reader&, size_t vertex_count);

               void parse_unknown_shorts(file_reader&);
               static void skip_unknown_shorts(file_reader&);

               void parse_vertex_colors(file_reader&, size_t vertex_count);
               static void skip_vertex_colors(file_reader&, size_t vertex_count);

               void parse_uv_sets(file_reader&, size_t vertex_count, bool has_uv);
               static void skip_uv_sets(file_reader&, size_t vertex_count, bool has_uv, bool uses_abbreviated_vertices);
         };

      public:
         //
         // Fields that NifSkope defines in NiGeometryData:
         //
         int32_t  group_id = 0; // always 0
         uint16_t max_particle_count = 0;
         struct {
            uint8_t keep = 0;
            uint8_t compress = 0;
            NiGeometryData::vector_flags vector = 0;
         } flags;
         std::optional<geometry_data> geometry_data;
         uint32_t  material_crc = 0;
         NiBound   bounds;
         uint16_t  consistency_flags = 0;
         NiObject* additional = nullptr;
         //
         // Fields unique to NiParticlesData:
         //
         uint16_t active_particle_count = 0;
         //
         // Bethesda:
         //
         bool has_texture_indices = false;
         std::vector<std::array<float, 4>> subtexture_offsets;
         float    aspect_ratio = 1;
         uint16_t aspect_flags = 0;
         struct {
            float to_aspect_aspect_2 = 0;
            float to_aspect_speed_1  = 0;
            float to_aspect_speed_2  = 0;
         } speeds;

         virtual void parse(file_reader&) override;
   };
}