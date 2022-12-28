#pragma once
#include <array>
#include <numbers>
#include "helpers/math/geometry/generate_triangulated_n_gon_indices.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"
#include "../../enums/axis3D.h"
#include "../index_type.h"
#include "../vertex.h"

namespace vulkanDK {
   class  raycast;
   struct raycast_hit_data;
}

namespace vulkanDK::gizmos::meshes::translate {
   namespace options {
      constexpr float  stem_length      = 384;
      constexpr float  stem_radius      =   6;
      constexpr float  arrowhead_radius =  32;
      constexpr float  arrowhead_length =  96;
      constexpr size_t arrowhead_verts  =   6;
      constexpr bool   righthanded      = true;

      constexpr float raycast_inflation = 6;
   }

   namespace impl {
      constexpr auto stem_thickness = options::stem_radius * 2;

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

      static constexpr size_t vertices_per_arrowhead = options::arrowhead_verts + 1; // plus one for the tip
      static constexpr size_t indices_per_arrowhead  = cobb::geometry::triangle_indices_for_n_gon(options::arrowhead_verts) + (options::arrowhead_verts * 3);

      static constexpr size_t vertices_per_stem = x_stem_vertices.size();
      static constexpr size_t indices_per_stem  = x_stem_indices.size();

      static constexpr size_t vertices_per_axis = vertices_per_arrowhead + vertices_per_stem;
      static constexpr size_t indices_per_axis  = indices_per_arrowhead + indices_per_stem;
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
         // Generate the arrowhead:
         //
         {
            float rads_per = (2 * std::numbers::pi_v<float>) / (float)options::arrowhead_verts;
            for (size_t v = 0; v < options::arrowhead_verts; ++v) {
               float main  = options::stem_radius + options::arrowhead_radius * cobb::cosine(v * rads_per);
               float cross = options::stem_radius + options::arrowhead_radius * cobb::sine(v * rads_per);

               auto& vert = out.vertices[start_v + v];
               switch (axis) {
                  case 0: vert.position.x = options::stem_length; vert.position.y = main; vert.position.z = cross; break; // X arrowhead base would be on the YZ plane
                  case 1: vert.position.y = options::stem_length; vert.position.x = main; vert.position.z = cross; break; // Y arrowhead base would be on the XZ plane
                  case 2: vert.position.z = options::stem_length; vert.position.x = main; vert.position.y = cross; break; // Z arrowhead base would be on the XY plane
               }
               vert.position.w = axis;
            }
            cobb::geometry::generate_triangulated_n_gon_indices(options::arrowhead_verts, start_v, out.indices, start_i);
            start_i += cobb::geometry::triangle_indices_for_n_gon(options::arrowhead_verts);
            //
            // And generate the tip:
            //
            auto& tip = out.vertices[start_v + options::arrowhead_verts];
            tip.position[axis] = options::stem_length + options::arrowhead_length;
            tip.position.w = axis;
            //
            // Connect it to the base:
            //
            for (size_t v = 0; v < options::arrowhead_verts; ++v) {
               out.indices[start_i + (v * 3)]     = (start_v + v);
               out.indices[start_i + (v * 3) + 1] = (start_v + ((v + 1) % options::arrowhead_verts));
               out.indices[start_i + (v * 3) + 2] = (start_v + options::arrowhead_verts); // tip vertex
            }
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