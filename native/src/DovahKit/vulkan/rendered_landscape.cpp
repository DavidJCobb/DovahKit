#include "rendered_landscape.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/LandTexture.h"
#include "dovah/forms/TextureSet.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/norm.hpp>

#include <QDebug>

namespace vulkanDK {
   void rendered_landscape::_on_shader_parameter_change() {
      if (this->pending_delete())
         return;
      this->handled_frames.set_all_out_of_date();
   }

   void rendered_landscape::set_position(const glm::vec3& pos) {
      this->shader_params.position = pos;
      this->_on_shader_parameter_change();
   }

   void rendered_landscape::import_vertex_data_from_form(const loaded_form& land) {
      constexpr size_t centerline_index_src = 16;
      //
      auto& vl = this->vertices;
      for (size_t q = 0; q < 4; ++q) {
         size_t offset_x = (q % 2) * centerline_index_src;
         size_t offset_y = (q / 2) * centerline_index_src;
         //
         for (size_t y = 0; y < vertices_per_quad_side; ++y) {
            for (size_t x = 0; x < vertices_per_quad_side; ++x) {
               size_t src_i = ((y + offset_y) * loaded_form::vertices_per_side) + (x + offset_x);
               size_t dst_i = (q * vertices_per_quad) + (y * vertices_per_quad_side) + x;
               //
               const auto& color = land.heightmap.colors.by_flat_index(src_i);
               if (land.land_flags & loaded_form::land_flag::has_colors) {
                  vl[dst_i].color = glm::vec3{ (float)color.r, (float)color.g, (float)color.b } / 255.0F;
               } else {
                  vl[dst_i].color = glm::fvec3{ 1, 1, 1 };
               }
               //
               vl[dst_i].height = land.heightmap.heights.by_flat_index(src_i);
               for (size_t j = 0; j < vl[dst_i].blends.size(); ++j)
                  vl[dst_i].blends[j] = 0;
            }
         }
      }
      //
      // Normals:
      //
      {
         std::array<cobb::vector3<float>, loaded_form::total_vertex_count> normals;
         land.recalc_normals_to(normals);
         for (size_t q = 0; q < 4; ++q) {
            size_t offset_x = (q % 2) * centerline_index_src;
            size_t offset_y = (q / 2) * centerline_index_src;
            //
            for (size_t y = 0; y < vertices_per_quad_side; ++y) {
               for (size_t x = 0; x < vertices_per_quad_side; ++x) {
                  size_t src_i = ((y + offset_y) * loaded_form::vertices_per_side) + (x + offset_x);
                  size_t dst_i = (q * vertices_per_quad) + (y * vertices_per_quad_side) + x;
                  //
                  vl[dst_i].normal = normals[src_i].to_struct<glm::vec3>();
               }
            }
         }
      }
      //
      // Get alpha-blending data:
      //
      for (size_t q = 0; q < 4; ++q) {
         size_t offset_x = (q % 2) * centerline_index_src;
         size_t offset_y = (q / 2) * centerline_index_src;
         //
         for (auto& blend : land.alpha_layers_by_quad[q]) {
            auto layer = blend.layer;
            if (layer < 0 || layer >= rendered_landscape::max_usable_layers_per_quad)
               continue;
            //
            auto& alphas = blend.opacities;
            for (size_t y = 0; y < loaded_form::vertices_per_side; ++y) {
               for (size_t x = 0; x < loaded_form::vertices_per_side; ++x) {
                  auto f = alphas.item(x, y);
                  if (f <= 0)
                     continue;
                  //
                  auto i = (q * vertices_per_quad) + ((y - offset_y) * vertices_per_quad_side) + (x - offset_x);
                  vl[i].blends[layer] = f;
               }
            }
         }
      }
   }
   void rendered_landscape::setup_vertex_data_at(void* dest) {
      auto& vl = this->vertices;
      auto  vs = loaded_form::total_vertex_count * sizeof(vertex_landscape);
      memcpy((void*)dest, vl.data(), vs);
   }

   /*//
   [[nodiscard]] VkDrawIndexedIndirectCommand rendered_landscape::make_indirect_draw_command() const;
   //*/

   void rendered_landscape::mark_for_delete() {
      this->life_state = scene_frame_item_state::pending_delete;
      this->handled_frames.set_all_out_of_date();
   }
   /*//
   void rendered_landscape::reset();
   //*/

   glm::vec3 rendered_landscape::local_vertex_position(int quad, size_t quad_vertex_index) const {
      quad_vertex_index = quad_vertex_index % vertices_per_quad;
      int   land_x = quad_vertex_index % vertices_per_quad_side + ((vertices_per_quad_side - 1) * (quad % 2));
      int   land_y = quad_vertex_index / vertices_per_quad_side + ((vertices_per_quad_side - 1) * (quad / 2));
      float height = this->vertices[quad_vertex_index + (quad * vertices_per_quad)].height;
      //
      return glm::vec3(
         land_x * vertex_distance,
         land_y * vertex_distance,
         height
      );
   }
   glm::vec3 rendered_landscape::local_vertex_position(size_t mesh_vertex_index) const {
      return this->local_vertex_position(mesh_vertex_index / vertices_per_quad, mesh_vertex_index % vertices_per_quad);
   }
   glm::vec3 rendered_landscape::world_vertex_position(int quad, size_t quad_vertex_index) const {
      return this->local_vertex_position(quad, quad_vertex_index) + this->shader_params.position;
   }
   glm::vec3 rendered_landscape::world_vertex_position(size_t mesh_vertex_index) const {
      return this->world_vertex_position(mesh_vertex_index / vertices_per_quad, mesh_vertex_index % vertices_per_quad);
   }
   
   bool rendered_landscape::ray_intersects(const glm::vec3& ray_origin, glm::vec3 ray_direction, float& hit_distance) const {
      if (this->empty())
         return false;
      //
      constexpr auto fourth_index_per_quad = ([]() {
         auto a = quad_vertex_indices[0];
         auto b = quad_vertex_indices[1];
         auto c = quad_vertex_indices[2];
         for (int i = 3; i < 6; ++i) {
            auto d = quad_vertex_indices[i];
            if (d != a && d != b && d != c)
               return i;
         }
         throw;
      })();
      //
      ray_direction = glm::normalize(ray_direction);
      glm::vec2 bary_position;
      //
      auto& vert = this->vertices;
      bool  hits = false;
      hit_distance = std::numeric_limits<float>::max();
      for (size_t q = 0; q < 4; ++q) {
         static_assert(indices_per_quad % 6 == 0, "A quad is two triangles is six indices, and there should only be quads.");
         for (size_t i = 0; i + 5 < indices_per_quad; i += 6) {
            auto a = this->world_vertex_position(q, quad_vertex_indices[i + 0]);
            auto b = this->world_vertex_position(q, quad_vertex_indices[i + 1]);
            auto c = this->world_vertex_position(q, quad_vertex_indices[i + 2]);
            auto d = this->world_vertex_position(q, quad_vertex_indices[i + fourth_index_per_quad]);
            //
            float distance;
            bool  result = glm::intersectRayTriangle(
               ray_origin,
               ray_direction,
               a, b, c,
               bary_position,
               distance
            );
            if (!result || distance < 0) { // GLM didn't implement their math properly; you can get a false-positive result with a negative distance, meaning the "hit position" is behind the ray
               result = glm::intersectRayTriangle(
                  ray_origin,
                  ray_direction,
                  b, c, d,
                  bary_position,
                  distance
               );
               if (!result || distance < 0) // GLM didn't implement their math properly; you can get a false-positive result with negative distance, meaning the "hit position" is behind the ray
                  continue;
            }
            hits         = result;
            hit_distance = std::min(distance, hit_distance);
         }
      }
      return hits;
   }
}