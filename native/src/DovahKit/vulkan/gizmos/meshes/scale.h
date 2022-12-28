#pragma once
#include <array>
#include "../../enums/axis3D.h"
#include "../index_type.h"
#include "../vertex.h"

namespace vulkanDK {
   class  raycast;
   struct raycast_hit_data;
}

namespace vulkanDK::gizmos::meshes::scale {
   namespace options {
      constexpr float  stem_length  = 448;
      constexpr float  stem_radius  =   6;
      constexpr float  handle_width =  48;
      constexpr bool   righthanded  = true;

      constexpr float raycast_inflation = 6;
   }

   namespace impl {
      constexpr auto stem_thickness = options::stem_radius * 2;

      constexpr auto box_vertices = []() {
         auto list = std::array{
            glm::vec3{ -1, -1, -1 },
            glm::vec3{  1, -1, -1 },
            glm::vec3{ -1,  1, -1 },
            glm::vec3{  1,  1, -1 },
            glm::vec3{ -1, -1,  1 },
            glm::vec3{  1, -1,  1 },
            glm::vec3{ -1,  1,  1 },
            glm::vec3{  1,  1,  1 },
         };
         for (auto& item : list)
            item *= (options::handle_width / 2);
         return list;
      }();
      constexpr auto box_indices = []() {
         auto list = std::array{
            0, 2, 3,   0, 3, 1,
            2, 6, 7,   2, 7, 3,
            6, 4, 5,   6, 5, 7,
            4, 0, 1,   4, 1, 5,
            0, 4, 6,   0, 6, 2,
            1, 5, 7,   1, 7, 3,
         };
         if (!options::righthanded) {
            for (size_t i = 0; i < list.size(); i += 3) {
               std::swap(list[i], list[i + 2]);
            }
         }
         return list;
      }();

      constexpr auto x_stem_vertices = std::array{
         //
         // Near side:
         //
         glm::vec3{ 0, 0, 0 },
         glm::vec3{ stem_thickness, 0, stem_thickness },
         glm::vec3{ stem_thickness, stem_thickness, 0 },
         //
         // Far side:
         //
         glm::vec3{ options::stem_length + 1, 0, 0 },
         glm::vec3{ options::stem_length + 1, 0, stem_thickness },
         glm::vec3{ options::stem_length + 1, stem_thickness, 0 },
         //
         // Near side inner corner:
         //
         glm::vec3{ 0, stem_thickness, 0 },
      };

      constexpr auto x_stem_indices = std::array{
         0, 3, 4,
         4, 1, 0,

         0, 3, 5,
         5, 2, 0,

         4, 5, 6,
         6, 1, 4,
      };

      static constexpr size_t vertices_per_stem = x_stem_vertices.size();
      static constexpr size_t indices_per_stem  = x_stem_indices.size();

      static constexpr size_t vertices_per_axis = box_vertices.size() + vertices_per_stem;
      static constexpr size_t indices_per_axis  = box_indices.size() + indices_per_stem;
   }

   static constexpr const size_t vertex_count = impl::vertices_per_axis * 3;
   static constexpr const size_t index_count  = impl::indices_per_axis * 3;
   static constexpr const size_t vib_v_size   = sizeof(vertex) * vertex_count;
   static constexpr const size_t vib_i_size   = sizeof(index_type) * index_count;

   struct mesh_type {
      std::array<vertex,     vertex_count> vertices;
      std::array<index_type, index_count> indices;
   };

   constexpr mesh_type mesh = []() {
      mesh_type out = {};

      for (int axis = 0; axis < 3; ++axis) {
         auto start_v = (size_t)axis * impl::vertices_per_axis;
         auto start_i = (size_t)axis * impl::indices_per_axis;
         //
         // Generate the stem:
         //
         for (size_t i = 0; i < impl::vertices_per_stem; ++i) {
            auto& dst = out.vertices[start_v + i];
            auto& src = impl::x_stem_vertices[i];
            
            dst.position[axis] = src.x;
            dst.position[(axis + 1) % 3] = src.y;
            dst.position[(axis + 2) % 3] = src.z;
            dst.position.w = axis;
         }
         for (size_t i = 0; i < impl::indices_per_stem; ++i) {
            out.indices[start_i + i] = start_v + impl::x_stem_indices[i];
         }
         start_v += impl::vertices_per_stem;
         start_i += impl::indices_per_stem;
         //
         // Generate the handle:
         //
         for (size_t i = 0; i < impl::box_vertices.size(); ++i) {
            auto& dst = out.vertices[start_v + i];
            auto& src = impl::box_vertices[i];
            
            dst.position[axis] = src.x + options::stem_length + (options::handle_width / 2);
            dst.position[(axis + 1) % 3] = options::stem_radius + src.y;
            dst.position[(axis + 2) % 3] = options::stem_radius + src.z;
            dst.position.w = axis;
         }
         for (size_t i = 0; i < impl::box_indices.size(); ++i) {
            out.indices[start_i + i] = start_v + impl::box_indices[i];
         }
      }

      return out;
   }();

   extern raycast_hit_data do_raycast(
      const glm::mat4& transform,
      const raycast&,
      //
      axis3D& out_which_axis
   );
}