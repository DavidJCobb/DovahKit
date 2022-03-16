#include "NiSkinPartition.h"
#include "../reader.h"
#include "../types/Float16.h"

namespace nifDK::block_types {
   void NiSkinPartition::partition::read(file_reader& reader) {
      uint16_t count_vert;
      uint16_t count_tri;
      uint16_t count_bone;
      uint16_t count_strip;
      reader.read(count_vert);
      reader.read(count_tri);
      reader.read(count_bone);
      reader.read(count_strip);
      reader.read(this->weights_per_vertex);
      //
      this->bones.resize(count_bone);
      reader.read_vector_contents(this->bones);
      //
      uint8_t presence = (reader.version() < file_version::from_parts<10, 1, 0, 0>) ? 1 : 0;
      if (!presence) {
         reader.read(presence);
      }
      if (presence) {
         this->vertex_map.resize(count_vert);
         reader.read_vector_contents(this->vertex_map);
      }
      presence = (reader.version() < file_version::from_parts<10, 1, 0, 0>) ? 1 : 0;
      if (!presence) {
         reader.read(presence);
      }
      if (presence) {
         this->weights.resize(this->weights_per_vertex * count_vert);
         if (reader.version() >= file_version::from_parts<20, 3, 1, 1> && presence == 15) {
            reader.require_size(this->weights.size() * sizeof(uint16_t));
            for (auto& f : this->weights) {
               uint16_t half;
               reader.unchecked_read(half);
               f = Float16(half);
            }
         } else {
            reader.read_vector_contents(this->weights);
         }
      }
      //
      bool has_faces = reader.version() < file_version::from_parts<10, 1, 0, 0>;
      {
         std::vector<uint16_t> strip_lengths(count_strip);
         reader.read_vector_contents(strip_lengths);
         if (!has_faces) {
            reader.read(has_faces);
         }
         if (has_faces) {
            if (count_strip) {
               this->strips.resize(count_strip);
               for (auto& s : this->strips) {
                  s.resize(count_strip);
                  reader.read_vector_contents(s);
               }
            } else {
               auto& list = this->triangles;
               list.resize(count_tri);
               reader.require_size(count_tri * 3 * sizeof(decltype(Triangle::vertex_indices)::value_type));
               for (auto& t : list)
                  reader.unchecked_read(t.vertex_indices);
            }
         }
      }
      reader.read(presence);
      if (presence) {
         this->bone_indices.resize(count_vert * this->weights_per_vertex);
         reader.read_vector_contents(this->bone_indices);
      }
      if (reader.user_version<2>() > 34) {
         reader.read(this->unknown);
      }
      //
      bool is_remaster = reader.version() >= file_version::from_parts<20, 2, 0, 7> && reader.user_version<2>() == 100;
      if (is_remaster) {
         this->remaster.vertex_description.read(reader);
         //
         auto& list = this->remaster.triangles;
         list.resize(count_tri);
         reader.require_size(count_tri * 3 * sizeof(decltype(Triangle::vertex_indices)::value_type));
         for (auto& t : list)
            reader.unchecked_read(t.vertex_indices);
      }
   }

   void NiSkinPartition::parse(file_reader& reader) {
      bool is_remaster = reader.version() >= file_version::from_parts<20, 2, 0, 7> && reader.user_version<2>() == 100;

      uint32_t count;
      reader.read(count);
      if (is_remaster) {
         uint32_t data_size;
         uint32_t vertex_size;
         reader.read(data_size);
         reader.read(vertex_size);
         this->remaster.vertex_description.read(reader);
         //
         auto& list = this->remaster.vertices;
         list.resize(vertex_size / data_size);
         BSVertexDataSSE::parse_all(reader, this->remaster.vertex_description, list);
      }
      this->partitions.resize(count);
      for (uint32_t i = 0; i < count; ++i) {
         reader.read(this->partitions[i]);
      }
   }
}