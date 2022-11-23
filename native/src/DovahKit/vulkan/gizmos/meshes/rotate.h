#pragma once
#include <array>
#include <numbers>
#include "helpers/glm/constexpr.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"
#include "helpers/math/sqrt.h"
#include "../index_type.h"
#include "../vertex.h"

namespace vulkanDK::gizmos::meshes::rotate {
   namespace options {
      constexpr float  hoop_radius    = 489;
      constexpr float  hoop_thickness =  14;

      constexpr size_t loops_per_hoop    = 24; // roundness of the circle as a whole
      constexpr size_t vertices_per_loop =  3; // roundness of the tube which forms the circle

      constexpr bool   righthanded      = true;
   }

   namespace impl {
      static constexpr const float loop_thickness     = options::hoop_thickness / 2;
      static constexpr const float hoop_center_radius = options::hoop_radius - loop_thickness;

      static constexpr const size_t indices_per_loop  = options::vertices_per_loop * 2 * 3; // number of verts = number of quads; 1 quad = 2 tris; 1 tri = 3 indices

      static constexpr const size_t vertices_per_axis = options::loops_per_hoop * options::vertices_per_loop;
      static constexpr const size_t indices_per_axis  = options::loops_per_hoop * indices_per_loop;
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

      constexpr float radians_per_loop        = (2 * std::numbers::pi_v<float>) / (float)options::loops_per_hoop;
      constexpr float radians_per_loop_vertex = (2 * std::numbers::pi_v<float>) / (float)options::vertices_per_loop;

      for (int axis = 0; axis < 3; ++axis) {
         auto start_v = (size_t)axis * impl::vertices_per_axis;
         auto start_i = (size_t)axis * impl::indices_per_axis;

         glm::vec3 axis_vec;
         switch (axis) {
            case 0: axis_vec = { 1, 0, 0 }; break;
            case 1: axis_vec = { 0, 1, 0 }; break;
            case 2: axis_vec = { 0, 0, 1 }; break;
         }

         //
         // Generate vertices:
         //
         for (size_t loop = 0; loop < options::loops_per_hoop; ++loop) {
            glm::vec3 centerpoint = { 0, 0, 0 };
            {
               float rel_x = impl::hoop_center_radius * cobb::cosine(loop * radians_per_loop);
               float rel_y = impl::hoop_center_radius * cobb::sine(loop * radians_per_loop);
               switch (axis) {
                  case 0: centerpoint = { 0, rel_x, rel_y }; break;
                  case 1: centerpoint = { rel_x, 0, rel_y }; break;
                  case 2: centerpoint = { rel_x, rel_y, 0 }; break;
               }
            }
            
            glm::vec3 axis_main  = cobb::glm::normalize(centerpoint);
            glm::vec3 axis_cross = axis_vec;

            for (size_t i = 0; i < options::vertices_per_loop; ++i) {
               auto& dst = out.vertices[start_v + (loop * options::vertices_per_loop) + i];

               float rel_x = impl::loop_thickness * cobb::cosine(i * radians_per_loop_vertex);
               float rel_y = impl::loop_thickness * cobb::sine(i * radians_per_loop_vertex);

               dst.position = glm::vec4(centerpoint + (axis_main * rel_x) + (axis_cross * rel_y), axis);
            }
         }
         //
         // Generate indices:
         //
         for (size_t loop = 0; loop < options::loops_per_hoop; ++loop) {
            size_t first_v = start_v + (loop * options::vertices_per_loop);
            size_t next_v  = start_v + (((loop + 1) * options::vertices_per_loop) % impl::vertices_per_axis);
            size_t first_i = start_i + (loop * impl::indices_per_loop);
            
            constexpr const size_t indices_per_vert = 6;
            for (size_t i = 0; i < options::vertices_per_loop; ++i) {
               auto base_i = first_i + (i * indices_per_vert);
               out.indices[base_i + 0] = first_v + i;
               out.indices[base_i + 1] = first_v + (i + 1) % options::vertices_per_loop;
               out.indices[base_i + 2] = next_v  + (i + 1) % options::vertices_per_loop;
               out.indices[base_i + 3] = next_v  + (i + 1) % options::vertices_per_loop;
               out.indices[base_i + 4] = next_v  + i;
               out.indices[base_i + 5] = first_v + i;
            }
         }
      }

      return out;
   }();
}