//
#include <cassert>
#include <cstdint>
#include <numbers>
#include <vector>
#include <glm/glm.hpp>
#include "helpers/math/geometry/generate_triangulated_n_gon_indices.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"

//
// experimental code; make this more formal
//

namespace vulkanDK::predefined_meshes {

   constexpr float  stem_length = 384;
   constexpr float  stem_radius = 6;
   constexpr float  arrowhead_radius = 32;
   constexpr float  arrowhead_length = 96;
   constexpr size_t arrowhead_verts = 6;

   constexpr bool righthanded = true;

   struct _gizmo_vertex {
      glm::vec3 position = { 0, 0, 0 };
      glm::vec3 color    = { 255, 255, 255 };

      constexpr _gizmo_vertex() {}
      constexpr _gizmo_vertex(const glm::vec3& v) : position(v) {}
   };

   struct _gizmo_mesh {
      std::vector<_gizmo_vertex> vertices;
      std::vector<uint16_t> indices;

      constexpr void add_triangle(size_t a, size_t b, size_t c) {
         auto i = this->indices.size();
         this->indices.resize(i + 3);
         if constexpr (righthanded) {
            this->indices[i]     = a;
            this->indices[i + 1] = b;
            this->indices[i + 2] = c;
         } else {
            this->indices[i]     = c;
            this->indices[i + 1] = b;
            this->indices[i + 2] = a;
         }
      }

      constexpr void add_triangle(size_t basis, size_t a, size_t b, size_t c) {
         this->add_triangle(basis + a, basis + b, basis + c);
      }
   };

   static constexpr size_t vertices_per_arrowhead() { return arrowhead_verts + 1; } // plus one for the tip
   static constexpr size_t vertices_per_stem() { return 6; }

   static constexpr size_t indices_per_arrowhead() {
      //
      // Triangles in the arrowhead base (an N-gon) are (N - 2).
      // 
      // Triangles to connect to the tip are N.
      //
      return ((arrowhead_verts - 2) * 3) * (arrowhead_verts * 3);
   }
   static constexpr size_t triangles_per_arrowhead() {
      return 7;
   }

   constexpr _gizmo_mesh make_gizmo_translate() {
      _gizmo_mesh mesh;
      for (int axis = 0; axis < 3; ++axis) {
         //
         // Generate the stem:
         //
         {
            auto first_vertex = mesh.vertices.size() + 1;

            constexpr auto stem_thickness = stem_radius * 2;
            mesh.vertices.emplace_back(glm::vec3{ 0, 0, 0 });
            mesh.vertices.emplace_back(glm::vec3{ stem_thickness, 0, stem_thickness });
            mesh.vertices.emplace_back(glm::vec3{ stem_thickness, stem_thickness, 0 });
            mesh.vertices.emplace_back(glm::vec3{ stem_length + 1, 0, 0 });
            mesh.vertices.emplace_back(glm::vec3{ stem_length + 1, 0, stem_thickness });
            mesh.vertices.emplace_back(glm::vec3{ stem_length + 1, stem_thickness, 0 });
            mesh.vertices.emplace_back(glm::vec3{ 0, stem_thickness, stem_thickness });
            mesh.add_triangle(first_vertex, 0, 3, 4);
            mesh.add_triangle(first_vertex, 3, 4, 1);
            mesh.add_triangle(first_vertex, 0, 3, 5);
            mesh.add_triangle(first_vertex, 3, 5, 2);
            mesh.add_triangle(first_vertex, 7, 4, 5);
            mesh.add_triangle(first_vertex, 4, 5, 2);
            mesh.add_triangle(first_vertex, 1, 2, 7);
            //
            if (axis != 0) {
               for (size_t i = 0; i < 7; ++i) {
                  auto& v    = mesh.vertices[first_vertex + i];
                  auto  copy = v.position;
                  v.position[axis] = copy[0];
                  v.position[(axis + 1) % 3] = copy[1];
                  v.position[(axis + 2) % 3] = copy[2];
               }
            }
         }
         //
         // Generate the arrowhead:
         //
         {
            auto first_vertex = mesh.vertices.size() + 1;
            //
            float rads_per = (2 * std::numbers::pi_v<float>) / (float)arrowhead_verts;
            for (size_t v = 0; v < arrowhead_verts; ++v) {
               float main  = arrowhead_radius * cobb::cosine(v * rads_per);
               float cross = arrowhead_radius * cobb::sine(v * rads_per);

               auto& vert = mesh.vertices.emplace_back();
               switch (axis) {
                  case 0: vert.position.x = stem_length; vert.position.y = main; vert.position.z = cross; break; // X arrowhead base would be on the YZ plane
                  case 1: vert.position.y = stem_length; vert.position.x = main; vert.position.z = cross; break; // Y arrowhead base would be on the XZ plane
                  case 2: vert.position.z = stem_length; vert.position.x = main; vert.position.y = cross; break; // Z arrowhead base would be on the XY plane
               }
            }
            cobb::geometry::generate_triangulated_n_gon_indices(arrowhead_verts, first_vertex, mesh.indices);
            //
            // And generate the tip:
            //
            auto& tip = mesh.vertices.emplace_back();
            tip.position[axis] = stem_length + arrowhead_length;
            //
            // Connect it to the base:
            //
            for (size_t v = 0; v < arrowhead_verts; ++v) {
               mesh.indices.push_back(first_vertex + v);
               mesh.indices.push_back(first_vertex + ((v + 1) % arrowhead_verts));
               mesh.indices.push_back(mesh.vertices.size() - 1); // tip vertex
            }
         }
      }
      return mesh;
   }
}