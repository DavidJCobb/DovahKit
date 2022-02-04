#include "NiGeometryData.h"
#include "../reader.h"
#include "../types/Float16.h"

namespace nifDK::block_types {
   void NiGeometryData::parse(file_reader& reader) {
      reader.read(this->group_id);
      //
      uint8_t  presence;
      uint8_t  vertex_type = 0;
      uint16_t vertex_count;
      reader.read(vertex_count);
      reader.read(this->flags.keep);
      reader.read(this->flags.compress);
      //
      bool can_use_half_vertices = reader.version() >= file_version::from_parts<20, 3, 1, 1>;
      //
      reader.read(vertex_type); // Has Vertices
      if (vertex_type) {
         this->vertices.resize(vertex_count);
         if (can_use_half_vertices && vertex_type == 15) {
            reader.read_half_vector_contents(this->vertices);
         } else {
            reader.read_vector_contents(this->vertices);
         }
      }
      if (reader.version() == file_version::from_parts<20, 2, 0, 7>) {
         if (reader.user_version<2>() > 0) {
            reader.read(this->flags.vector);
         }
         if (reader.user_version<1>() >= 12) {
            reader.read(this->material_crc);
         }
      }
      //
      reader.read(presence); // Has Normals
      if (presence) {
         this->normals.resize(vertex_count);
         if (can_use_half_vertices && presence == 6) {
            reader.read_half_vector_contents(this->normals);
         } else {
            reader.read_vector_contents(this->normals);
         }
         //
         if (this->flags.vector & vector_flag::has_tangents) {
            this->tangents.resize(vertex_count);
            this->bitangents.resize(vertex_count);
            reader.read_vector_contents(this->tangents);
            reader.read_vector_contents(this->bitangents);
         }
      }
      bool use_unknown_values = (reader.version() >= file_version::from_parts<20, 3, 0, 9> && (reader.user_version<1>() == 0x20000 || reader.user_version<1>() == 0x30000));
      if (use_unknown_values) {
         reader.read(presence);
         if (presence) {
            this->unknown_floats.resize(vertex_count);
            reader.read_vector_contents(this->unknown_floats);
         }
      }
      reader.read(this->bounds);
      if (use_unknown_values) {
         reader.read(this->unknown_shorts);
      }
      //
      reader.read(presence); // Has Vertex Colors
      if (presence) {
         this->vertex_colors.resize(vertex_count);
         reader.read_vector_contents(this->vertex_colors);
      }
      uint16_t uv_set_count = 0;
      reader.read(uv_set_count);
      if (this->flags.vector & vector_flag::has_uv) {
         auto count = (uv_set_count & 63) | 1;
         this->uv_sets.resize(count);
         for (size_t i = 0; i < count; ++i) {
            auto& set = this->uv_sets[i];
            set.resize(vertex_count);
            if (can_use_half_vertices && vertex_type == 15) {
               reader.read_half_vector_contents(set);
            } else {
               reader.read_vector_contents(set);
            }
         }
      }
      reader.read(this->consistency_flags);
      reader.read_ref(this->additional);
   }
}