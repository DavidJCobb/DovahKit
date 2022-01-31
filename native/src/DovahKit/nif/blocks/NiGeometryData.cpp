#include "NiGeometryData.h""
#include "../reader.h"

namespace nifDK::block_types {
   void NiGeometryData::parse(file_reader& reader) {
      reader.read(this->group_id);
      //
      uint8_t  presence;
      uint16_t vertex_count;
      reader.read(vertex_count);
      reader.read(this->flags.keep);
      reader.read(this->flags.compress);
      //
      //
      reader.read(presence); // Has Vertices
      if (presence) {
         this->vertices.resize(vertex_count);
         //
         // TODO: in version 20.3.1.1+, if (presence == 15), use "half-vectors."
         //
         reader.read_vector_vector(this->vertices);
      }
      if (true) { // TODO: version 20.2.0.7 and user version 2 > 0
         reader.read(this->flags.vector);
      }
      reader.read(this->material_crc); // TODO: 20.2.0.7 only, user version 12
      //
      reader.read(presence); // Has Normals
      if (presence) {
         this->normals.resize(vertex_count);
         //
         // TODO: in version 20.3.1.1+, if (presence == 6), use "half-vectors."
         //
         reader.read(this->normals.data(), vertex_count * sizeof(float) * 3);
         //
         if (this->flags.vector & vector_flag::has_tangents) {
            this->tangents.resize(vertex_count);
            this->bitangents.resize(vertex_count);
            reader.read_vector_vector(this->tangents);
            reader.read_vector_vector(this->bitangents);
         }
      }
      if (true) { // version 20.3.0.9; user version 1 == 0x20000 or 0x30000
         reader.read(presence);
         if (presence) {
            this->unknown_floats.resize(vertex_count);
            reader.read_vector(this->unknown_floats);
         }
      }
      reader.read(this->bounds);
      if (true) { // version 20.3.0.9; user version 1 == 0x20000 or 0x30000
         reader.read(this->unknown_shorts);
      }
      //
      reader.read(presence); // Has Vertex Colors
      if (presence) {
         this->vertex_colors.resize(vertex_count);
         reader.read_vector(this->vertex_colors);
      }
      uint16_t uv_set_count = 0;
      reader.read(uv_set_count);
      if (this->flags.vector & vector_flag::has_uv) {
         // TODO: if "Has Vertices" is 15, use a "half-vector"
         auto count = (uv_set_count & 63) | 1;
         this->uv_sets.resize(count);
         for (size_t i = 0; i < count; ++i) {
            auto& set = this->uv_sets[i];
            set.resize(vertex_count);
            reader.read_vector_vector(set);
         }
      }
      reader.read(this->consistency_flags);
      reader.read_ref(this->additional);
   }
}