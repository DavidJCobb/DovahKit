#pragma once
#include <type_traits>
#include "helpers/dynamic_fast_cast.h"
#include "nif/blocks/BSEffectShaderProperty.h"
#include "nif/blocks/BSLightingShaderProperty.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiTriShapeData.h"
#include "nif/types/SkyrimShaderPropertyFlags.h"
#include "../../rendered_mesh.h"

//
// October 18th, 2022 --- UNUSED
// 
// A variation on the code that `surface_renderer` uses to copy vertex data out of 
// a NiTriShapeData  and into the vertex list for a  `rendered_mesh`. We typically 
// have to branch during the per-vertex loop,  as a NIF may not contain all vertex 
// attributes. This variation on the usual code does compile-time branching within 
// a function templated  on every possible  combination of attributes,  instead of 
// run-time branching.
// 
// Testing indicates that  this code is barely faster  than the run-time branching 
// version when  dealing with large workloads.  In a test involving  793 NIFs, the 
// run-time-branching version took roughly 750ms total; this version, 732ms.
// 
// Conventional wisdom holds that branching has a performance cost. However, newer 
// CPUs have very  powerful "branch prediction"  functionality.  Evidently, branch 
// prediction  is good  enough (at least on my PC) for the  run-time  branching to 
// have nearly no impact.
// 
// There's another factor to consider when analyzing performance: memory locality, 
// or "how close together in memory are all the things the CPU needs to use?" This 
// applies to both data and code. When we use compile-time branching, we'll end up 
// splitting one function into several  separate variations, and all of these will 
// be located away from the call site.
// 
// As such,  it seems the optimal approach  is to just do run-time  branching. The 
// code here has been retained for reference,  but probably won't be kept updated.
//

namespace vulkanDK::helpers::mesh {
   namespace impl::_vertices_from_NiTriShapeData {
      struct permutation {
         bool enables_alpha;  // 2 possible values
         bool enables_colors; //
         bool has_colors;     // collectively, 3 possible values (neither; enables; enables and has)
         bool has_normals;    //
         bool has_tangents;   // collectively, 3 possible values (neither; normals; normals and tangents)
         bool has_uv_sets;    // 2 possible values
         //
         // Multiply each field's number of possible values together to get:
         //
         static constexpr const size_t possible_permutations = 2 * 3 * 3 * 2;

         constexpr int id() const noexcept {
            int id = 0;
            id +=             ((int)enables_alpha);
            id += 2 *         (has_colors  ? (int)has_colors  + (int)enables_colors : 0);
            id += 2 * 3 *     (has_normals ? (int)has_normals + (int)has_tangents   : 0);
            id += 2 * 3 * 3 * ((int)has_uv_sets);
            return id;
         }
         static constexpr permutation from_id(int id) noexcept {
            permutation out;
            //
            out.enables_alpha = (id    )         % 2;
            int colors        = (id / 2)         % 3;
            int normals       = (id / 2 / 3)     % 3;
            out.has_uv_sets   = (id / 2 / 3 / 3) % 2;
            //
            out.has_colors     = colors != 0;
            out.enables_colors = (colors & 2) != 0;
            //
            out.has_normals  = normals != 0;
            out.has_tangents = (normals & 2) != 0;
            //
            return out;
         }
      };

      template<
         permutation Settings
      >
      void copy(
         const nifDK::block_types::NiTriShapeData& block,
         rendered_mesh& mesh
      ) {
         size_t size = block.vertices.size();
         for (size_t i = 0; i < size; ++i) {
            auto& vert = mesh.mesh_data.vertices[i];
            vert.pos = block.vertices[i];
            if constexpr (Settings.has_colors) {
               auto& src = block.vertex_colors[i];
               if constexpr (Settings.enables_alpha) {
                  vert.color = { src.r, src.g, src.b, src.a };
               } else {
                  vert.color = { src.r, src.g, src.b, 1.0F };
               }
            } else {
               if constexpr (Settings.enables_colors) {
                  //
                  // If the shader enables vertex colors but the mesh data doesn't actually have any, 
                  // then color it all black.
                  //
                  vert.color = { 0.0, 0.0, 0.0, 1.0 };
               } else {
                  vert.color = { 1.0, 1.0, 1.0, 1.0 };
               }
            }
            if constexpr (Settings.has_uv_sets) {
               vert.uv = block.uv_sets[0][i];
            } else {
               vert.uv = { 0, 0 };
            }
            if constexpr (Settings.has_normals) {
               vert.normal = block.normals[i];
               if constexpr (Settings.has_tangents) {
                  vert.tangent   = block.tangents[i];
                  vert.bitangent = block.bitangents[i];
               } else {
                  vert.tangent   = { 1, 0, 0 };
                  vert.bitangent = { 0, 1, 0 };
               }
            } else {
               vert.normal    = { 0, 0, 1 };
               vert.tangent   = { 1, 0, 0 };
               vert.bitangent = { 0, 1, 0 };
            }
         }
      }

      using copy_function_type = decltype(&copy<permutation{}>);

      constexpr auto all_copy_functions = []() {
         std::array<copy_function_type, permutation::possible_permutations> out = {};
         [&out]<std::size_t... Indices>(std::index_sequence<Indices...>){
            (
               (out[Indices] = &copy<permutation::from_id(Indices)>),
               ...
            );
         }(std::make_index_sequence<permutation::possible_permutations>{});
         return out;
      }();
   }

   void vertices_from_NiTriShapeData(
      const nifDK::block_types::NiGeometry& geom,
      const nifDK::block_types::NiTriShapeData& block,
      rendered_mesh& mesh
   ) {
      using namespace nifDK::block_types;
      constexpr const bool generate_accurate_tangent_spaces = false;

      bool has_colors   = !block.vertex_colors.empty();
      bool has_normals  = !block.normals.empty();
      bool has_tangents = !block.tangents.empty() && !block.bitangents.empty();
      bool has_uv_sets  = !block.uv_sets.empty();

      bool enables_alpha  = false;
      bool enables_colors = false;

      std::array<nifDK::SkyrimShaderPropertyFlags, 2> shader_flags = { 0, 0 };
      if (const auto* shader = geom.properties.shader) {
         if (const auto* blsp = cobb::dynamic_fast_cast<const BSLightingShaderProperty*>(shader)) {
            shader_flags = blsp->shader_flags;
         } else if (const auto* besp = cobb::dynamic_fast_cast<const BSEffectShaderProperty*>(shader)) {
            shader_flags = besp->shader_flags;
         }
      }
      if (shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::vertex_alpha) {
         enables_alpha = true;
      }
      if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::vertex_colors) {
         enables_colors = true;
      }

      const auto size = block.vertices.size();
      mesh.mesh_data.vertices.resize(size);

      //
      // Copy over non-missing data:
      //

      using namespace impl::_vertices_from_NiTriShapeData;
      auto id = (permutation{
         .enables_alpha  = enables_alpha,
         .enables_colors = enables_colors,
         .has_colors     = has_colors,
         .has_normals    = has_normals,
         .has_tangents   = has_tangents,
         .has_uv_sets    = has_uv_sets,
      }).id();
      (all_copy_functions[id])(block, mesh);

      //
      // Generate missing data:
      //

      if (!has_tangents) {
         if constexpr (generate_accurate_tangent_spaces) {
            if (has_normals && has_uv_sets) {
               //
               // Generate the (bi)tangents using the normals and UVs.
               //
               // Per: https://web.archive.org/web/20140728205933/http://www.terathon.com/code/tangent.html
               // Lengyel, Eric. “Computing Tangent Space Basis Vectors for an Arbitrary Mesh”. Terathon Software 3D Graphics Library, 2001.
               //
               auto& tris = block.triangles;
               for (const auto& tri : tris) {
                  const auto ia = tri.vertex_indices[0];
                  const auto ib = tri.vertex_indices[1];
                  const auto ic = tri.vertex_indices[2];

                  const auto& va = block.vertices[ia];
                  const auto& vb = block.vertices[ib];
                  const auto& vc = block.vertices[ic];

                  const auto& wa = block.uv_sets[0][ia];
                  const auto& wb = block.uv_sets[0][ib];
                  const auto& wc = block.uv_sets[0][ic];

                  const auto uv_diff_1 = glm::vec2{ wb.x - wa.x, wc.x - wa.x };
                  const auto uv_diff_2 = glm::vec2{ wb.y - wa.y, wc.y - wa.y };
                  float scale = 1.0F / ( uv_diff_1.x * uv_diff_2.y - uv_diff_2.x * uv_diff_1.y );

                  std::array<float, 2> x;
                  std::array<float, 2> y;
                  std::array<float, 2> z;
                  for (int i = 0; i < 2; ++i) {
                     x[i] = block.vertices[tri.vertex_indices[i]].x - va.x;
                     y[i] = block.vertices[tri.vertex_indices[i]].y - va.y;
                     z[i] = block.vertices[tri.vertex_indices[i]].z - va.z;
                  }
                  const auto tangent = glm::vec3{
                     (uv_diff_2.y * x[0] - uv_diff_2.x * x[1]) * scale,
                     (uv_diff_2.y * y[0] - uv_diff_2.x * y[1]) * scale,
                     (uv_diff_2.y * z[0] - uv_diff_2.x * z[1]) * scale,
                  };
                  const auto bitangent = glm::vec3{
                     (uv_diff_1.x * x[1] - uv_diff_1.y * x[0]) * scale,
                     (uv_diff_1.x * y[1] - uv_diff_1.y * y[0]) * scale,
                     (uv_diff_1.x * z[1] - uv_diff_1.y * z[0]) * scale,
                  };

                  mesh.mesh_data.vertices[ia].tangent += tangent;
                  mesh.mesh_data.vertices[ib].tangent += tangent;
                  mesh.mesh_data.vertices[ic].tangent += tangent;
                  mesh.mesh_data.vertices[ia].bitangent += bitangent;
                  mesh.mesh_data.vertices[ib].bitangent += bitangent;
                  mesh.mesh_data.vertices[ic].bitangent += bitangent;
               }
               for (auto& vert : mesh.mesh_data.vertices) {
                  vert.tangent = glm::normalize(vert.tangent);
                  vert.tangent = (vert.tangent - vert.normal * glm::dot(vert.normal, vert.tangent));
                  vert.tangent = glm::normalize(vert.tangent);

                  vert.bitangent = glm::normalize(vert.bitangent);
                  vert.bitangent = (vert.bitangent - vert.normal * glm::dot(vert.normal,  vert.bitangent));
                  vert.bitangent = (vert.tangent   - vert.normal * glm::dot(vert.tangent, vert.bitangent));
                  vert.bitangent = glm::normalize(vert.bitangent);
               }
            }
         }
      }
   }
}