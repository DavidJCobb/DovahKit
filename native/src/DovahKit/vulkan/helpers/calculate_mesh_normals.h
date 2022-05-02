#pragma once
#include "helpers/glm/cross_simd.h"
#include "helpers/simd/sum_register.h"
#include "helpers/cpuinfo.h"
#include <vector>
#include <glm/glm.hpp>

namespace vulkanDK {
   template<bool equal_weights, typename Index> void calculate_mesh_normals(const std::vector<glm::vec3>& verts, const std::vector<Index> indices, std::vector<glm::vec3>& normals) {
      auto size = verts.size();
      normals.resize(size);
      //
      /*//
      if (cobb::cpuinfo::get().extension_support.sse_2) {
         constexpr sse3 = false;
         //
         auto _load_vertex = [](const std::vector<glm::vec3>& list, size_t i) {
            auto& vert = list[i];
            if (i + 1 < list.size()) {
               auto p = _mm_loadu_ps(&vert.x);
               return p;
            }
            auto x  = _mm_load_ss(&vert.x);
            auto y  = _mm_load_ss(&vert.y);
            auto z  = _mm_load_ss(&vert.z);
            auto xy = _mm_movelh_ps(x, y);
            return _mm_shuffle_ps(xy, z, _MM_SHUFFLE(2, 0, 2, 0));
         };
         auto _save_vertex_unordered = [](glm::vec3& out, __m128 v) {
            //
            // Could write to vertices in any order, so we can't just bulldoze part of the next vert in the list.
            //
            glm::vec4 temp;
            _mm_storeu_ps(&temp.x, v);
            out = temp;
         };
         auto _save_vertex_ordered = [](std::vector<glm::vec3>& list, size_t i, __m128 v) {
            auto& vert = list[i];
            if (i + 1 < list.size()) {
               _mm_storeu_ps(&vert.x, v);
               return;
            }
            _save_vertex_unordered(vert, v);
         };
         //
         for (size_t i = 0; i < indices.size(); ++i) {
            auto a = _load_vertex(verts, indices[i + 0]);
            auto b = _load_vertex(verts, indices[i + 1]);
            auto c = _load_vertex(verts, indices[i + 2]);
            //
            auto ab = _mm_sub_ps(b, a);
            auto ac = _mm_sub_ps(c, a);
            __m128 normal = cobb::glm::cross_simd_to_register(ab, ac);
            if constexpr (equal_weights) {
               normal = _mm_mul_ps(normal, _mm_set_ps(0, 1, 1, 1)); // clear W-component
               //
               auto length = _mm_sqrt_ps(cobb::simd::sum_register_to_register<sse3>(normal));
               if constexpr (!sse3) {
                  //
                  // Only the lowest element will have the right value.
                  //
                  length = _mm_shuffle_ps(length, length, _MM_SHUFFLE(0, 0, 0, 0));
               }
               normal = _mm_div_ps(normal, length);
            }
            _save_vertex_unordered(normals[indices[i]], normal);
         }
         for(size_t i = 0; i < normals.size(); ++i) {
            auto normal = _load_vertex(normals, i);
            //
            // Normalize:
            //
            normal = _mm_mul_ps(normal, _mm_set_ps(0, 1, 1, 1)); // clear W-component
            //
            auto length = _mm_sqrt_ps(cobb::simd::sum_register_to_register<sse3>(normal));
            if constexpr (!sse3) {
               //
               // Only the lowest element will have the right value.
               //
               length = _mm_shuffle_ps(length, length, _MM_SHUFFLE(0, 0, 0, 0));
            }
            normal = _mm_div_ps(normal, length);
            //
            _save_vertex_ordered(normals, i, normal);
         }
      } else {
      //*/
         for (size_t i = 0; i < indices.size(); ++i) {
            const auto& a = verts[indices[i + 0]];
            const auto& b = verts[indices[i + 1]];
            const auto& c = verts[indices[i + 2]];
            auto normal = glm::cross(b - a, c - a);
            if constexpr (equal_weights) {
               normal = glm::normalize(normal);
            }
            normals[i] += normal;
         }
         for (auto& n : normals)
            n = glm::normalize(n);
      /*//
      }
      //*/
   }
}