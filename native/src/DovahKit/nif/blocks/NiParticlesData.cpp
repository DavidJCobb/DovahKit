#include "NiParticlesData.h"
#include "../reader.h"
#include "../types/Float16.h"
#include "NiPSysData.h"

namespace nifDK::block_types {
   #pragma region NiParticlesData::geometry_data
      /*static*/ bool NiParticlesData::geometry_data::can_use_abbreviated_vertices(const file_reader& reader) {
         return reader.version() >= file_version::from_parts<20, 3, 1, 1>;
      }
      /*static*/ bool NiParticlesData::geometry_data::has_unknown_values(const file_reader& reader) {
         return (reader.version() >= file_version::from_parts<20, 3, 0, 9> && (reader.user_version<1>() == 0x20000 || reader.user_version<1>() == 0x30000));
      }

      void NiParticlesData::geometry_data::parse_vertices(file_reader& reader, size_t vertex_count) {
         uint8_t vertex_type;
         reader.read(vertex_type);
         if (vertex_type == 0)
            return;
         auto& dst = this->vertices;
         dst.resize(vertex_count);
         if (vertex_type == 15 && can_use_abbreviated_vertices(reader)) {
            this->uses_abbreviated_vertices = true;
            reader.read_half_vector_contents(dst);
         } else {
            reader.read_vector_contents(dst);
         }
      }
      /*static*/ void NiParticlesData::geometry_data::skip_vertices(file_reader& reader, size_t vertex_count, bool& uses_abbreviated_vertices) {
         uint8_t vertex_type;
         reader.read(vertex_type);
         if (vertex_type == 0)
            return;
         if (vertex_type == 15 && can_use_abbreviated_vertices(reader)) {
            uses_abbreviated_vertices = true;
            reader.skip(vertex_count * 3 * sizeof(uint16_t));
         } else {
            reader.skip(vertex_count * 3 * sizeof(float));
         }
      }

      void NiParticlesData::geometry_data::parse_normals(file_reader& reader, size_t vertex_count, bool has_tangents) {
         uint8_t normals_type;
         reader.read(normals_type);
         if (normals_type == 0)
            return;

         auto& dst = this->normals;
         dst.resize(vertex_count);
         if (normals_type == 6 && can_use_abbreviated_vertices(reader)) {
            uint8_t byte;
            reader.require_size(vertex_count * 3 * sizeof(byte));
            for (auto& n : dst) {
               reader.unchecked_read(byte);
               n.x = byte;
               reader.unchecked_read(byte);
               n.y = byte;
               reader.unchecked_read(byte);
               n.z = byte;
            }
         } else {
            reader.read_vector_contents(dst);
         }

         if (has_tangents) {
            this->tangents.resize(vertex_count);
            this->bitangents.resize(vertex_count);
            reader.read_vector_contents(this->tangents);
            reader.read_vector_contents(this->bitangents);
         }
      }
      /*static*/ void NiParticlesData::geometry_data::skip_normals(file_reader& reader, size_t vertex_count, bool has_tangents) {
         uint8_t normals_type;
         reader.read(normals_type);
         if (normals_type == 0)
            return;

         if (normals_type == 6 && can_use_abbreviated_vertices(reader)) {
            reader.skip(vertex_count * 3 * sizeof(uint8_t));
         } else {
            reader.skip(vertex_count * 3 * sizeof(float));
         }

         if (has_tangents) {
            reader.skip(vertex_count * 3 * sizeof(float)); // tangents
            reader.skip(vertex_count * 3 * sizeof(float)); // bitangents
         }
      }

      void NiParticlesData::geometry_data::parse_unknown_floats(file_reader& reader, size_t vertex_count) {
         if (!has_unknown_values(reader))
            return;
         bool presence;
         reader.read(presence);
         if (!presence)
            return;
         auto& dst = this->unknown_floats;
         dst.resize(vertex_count);
         reader.read_vector_contents(dst);
      }
      /*static*/ void NiParticlesData::geometry_data::skip_unknown_floats(file_reader& reader, size_t vertex_count) {
         if (!has_unknown_values(reader))
            return;
         bool presence;
         reader.read(presence);
         if (!presence)
            return;
         reader.skip(vertex_count * 3 * sizeof(float));
      }

      void NiParticlesData::geometry_data::parse_unknown_shorts(file_reader& reader) {
         if (has_unknown_values(reader))
            reader.read(this->unknown_shorts);
      }
      /*static*/ void NiParticlesData::geometry_data::skip_unknown_shorts(file_reader& reader) {
         using list_type = decltype(unknown_shorts);
         if (has_unknown_values(reader))
            reader.skip(sizeof(typename list_type::value_type) * std::tuple_size_v<list_type>);
      }

      void NiParticlesData::geometry_data::parse_vertex_colors(file_reader& reader, size_t vertex_count) {
         uint8_t presence;
         reader.read(presence);
         if (presence == 0)
            return;
         auto& dst = this->vertex_colors;
         dst.resize(vertex_count);
         if (presence == 7 && can_use_abbreviated_vertices(reader)) {
            uint8_t byte;
            reader.require_size(vertex_count * 4 * sizeof(byte));
            for (auto& n : dst) {
               reader.unchecked_read(byte);
               n.r = (float)byte / 255.0;
               reader.unchecked_read(byte);
               n.g = (float)byte / 255.0;
               reader.unchecked_read(byte);
               n.b = (float)byte / 255.0;
               reader.unchecked_read(byte);
               n.a = (float)byte / 255.0;
            }
         } else {
            reader.read_vector_contents(dst);
         }
      }
      /*static*/ void NiParticlesData::geometry_data::skip_vertex_colors(file_reader& reader, size_t vertex_count) {
         uint8_t presence;
         reader.read(presence);
         if (presence == 0)
            return;
         if (presence == 7 && can_use_abbreviated_vertices(reader)) {
            reader.skip(vertex_count * 4 * sizeof(uint8_t));
         } else {
            reader.skip(vertex_count * 4 * sizeof(float));
         }
      }

      void NiParticlesData::geometry_data::parse_uv_sets(file_reader& reader, size_t vertex_count, bool has_uv) {
         uint16_t uv_set_count = 0;
         if (reader.version() <= file_version::from_parts<4, 2, 2, 0>) {
            reader.read(uv_set_count);
            bool presence;
            reader.read(presence); // unused?
         }
         if (!has_uv)
            return;
         auto count = (uv_set_count & 63) | 1;
         auto& dst  = this->uv_sets;
         dst.resize(count);
         for (size_t i = 0; i < count; ++i) {
            auto& set = dst[i];
            set.resize(vertex_count);
            if (this->uses_abbreviated_vertices) {
               reader.read_half_vector_contents(set);
            } else {
               reader.read_vector_contents(set);
            }
         }
      }
      /*static*/ void NiParticlesData::geometry_data::skip_uv_sets(file_reader& reader, size_t vertex_count, bool has_uv, bool uses_abbreviated_vertices) {
         uint16_t uv_set_count = 0;
         if (reader.version() <= file_version::from_parts<4, 2, 2, 0>) {
            reader.read(uv_set_count);
            bool presence;
            reader.read(presence); // unused?
         }
         if (!has_uv)
            return;
         auto count = (uv_set_count & 63) | 1;
         if (uses_abbreviated_vertices) {
            reader.skip(count * vertex_count * 2 * sizeof(uint16_t));
         } else {
            reader.skip(count * vertex_count * 2 * sizeof(float));
         }
      }
   #pragma endregion

   void NiParticlesData::parse(file_reader& reader) {
      bool subclasses_ni_geometry_data = reader.version() != file_version::from_parts<20, 2, 0, 7> || reader.user_version<2>() <= 0;

      if (reader.version() >= file_version::from_parts<10, 1, 0, 114>) {
         reader.read(this->group_id);
      }
      reader.read(this->max_particle_count);
      reader.read(this->flags.keep);
      reader.read(this->flags.compress);

      auto vertex_count = this->max_particle_count;
      if (dynamic_cast<NiPSysData*>(this) && reader.user_version<2>() >= 34) {
         vertex_count = 0;
      } else {
         this->geometry_data.emplace();
      }

      bool skipped_geo_uses_abbreviated_vertices;
      if (this->geometry_data.has_value()) {
         this->geometry_data.value().parse_vertices(reader, vertex_count);
      } else {
         geometry_data::skip_vertices(reader, 0, skipped_geo_uses_abbreviated_vertices);
      }
      if (reader.version() >= file_version::from_parts<10, 0, 1, 0>) {
         reader.read(this->flags.vector);
      }
      if (reader.version() == file_version::from_parts<20, 2, 0, 7>) {
         if (reader.user_version<1>() >= 12) {
            reader.read(this->material_crc);
         }
      }
      if (this->geometry_data.has_value()) {
         auto& dst = this->geometry_data.value();
         dst.parse_normals(reader, vertex_count, (this->flags.vector & NiGeometryData::vector_flag::has_tangents));
         dst.parse_unknown_floats(reader, vertex_count);
      } else {
         geometry_data::skip_normals(reader, 0, (this->flags.vector & NiGeometryData::vector_flag::has_tangents));
         geometry_data::skip_unknown_floats(reader, vertex_count);
      }
      reader.read(this->bounds);
      if (this->geometry_data.has_value()) {
         auto& dst = this->geometry_data.value();
         dst.parse_unknown_shorts(reader);
         dst.parse_vertex_colors(reader, vertex_count);
         dst.parse_uv_sets(reader, vertex_count, (this->flags.vector & NiGeometryData::vector_flag::has_uv));
      } else {
         geometry_data::skip_unknown_shorts(reader);
         geometry_data::skip_vertex_colors(reader, vertex_count);
         geometry_data::skip_uv_sets(reader, vertex_count, (this->flags.vector & NiGeometryData::vector_flag::has_uv), skipped_geo_uses_abbreviated_vertices);
      }
      reader.read(this->consistency_flags);
      reader.read_ref(this->additional);
      //
      // NiParticlesData unique fields:
      //
      if (reader.version() <= file_version::from_parts<4, 0, 0, 2>) {
         reader.skip(sizeof(uint16_t)); // Num Particles
      }
      if (reader.version() <= file_version::from_parts<10, 0, 1, 0>) {
         reader.skip(sizeof(float)); // Particle Radius
      }
      {
         bool presence; // Has Radii
         reader.read(presence);
         if (presence && reader.version() >= file_version::from_parts<10, 1, 0, 0> && subclasses_ni_geometry_data) {
            reader.skip(vertex_count * sizeof(float)); // Radii
         }
      }
      reader.read(this->active_particle_count);
      {
         bool presence; // Has Sizes
         reader.read(presence);
         if (presence && reader.version() >= file_version::from_parts<10, 1, 0, 0> && subclasses_ni_geometry_data) {
            reader.skip(vertex_count * sizeof(float)); // Sizes
         }
      }
      {
         bool presence; // Has Rotations
         reader.read(presence);
         if (presence && reader.version() >= file_version::from_parts<10, 1, 0, 0> && subclasses_ni_geometry_data) {
            reader.skip(vertex_count * sizeof(float) * 4); // Rotations
         }
      }
      {
         bool presence; // Has Rotation Angles
         reader.read(presence);
         if (presence && reader.version() >= file_version::from_parts<10, 1, 0, 0> && subclasses_ni_geometry_data) {
            reader.skip(vertex_count * sizeof(float)); // Rotation Angles
         }
      }
      {
         bool presence; // Has Rotation Axes
         reader.read(presence);
         if (presence && reader.version() >= file_version::from_parts<10, 1, 0, 0> && subclasses_ni_geometry_data) {
            reader.skip(vertex_count * sizeof(float) * 3); // Rotation Axes
         }
      }
      if (!subclasses_ni_geometry_data) {
         reader.read(this->has_texture_indices);
      }
      {
         uint32_t count = 0;
         if (reader.user_version<2>() > 34) {
            reader.read(count);
         } else if (reader.version() == file_version::from_parts<20, 2, 0, 7>) {
            uint8_t b;
            reader.read(b);
            count = b;
         }
         if (!subclasses_ni_geometry_data) {
            this->subtexture_offsets.resize(count);
            for (auto& item : this->subtexture_offsets)
               reader.read(item);
         }
      }
      if (reader.user_version<2>() > 34) {
         reader.read(this->aspect_ratio);
         reader.read(this->aspect_flags);
         reader.read(this->speeds.to_aspect_aspect_2);
         reader.read(this->speeds.to_aspect_speed_1);
         reader.read(this->speeds.to_aspect_speed_2);
      }
   }
}