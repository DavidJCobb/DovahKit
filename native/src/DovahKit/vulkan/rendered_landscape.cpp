#include "rendered_landscape.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/LandTexture.h"
#include "dovah/forms/TextureSet.h"

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

   // World-relative raycasts (uses the mesh's transform):
   bool rendered_landscape::ray_intersects(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& hit_distance) const;
   //*/
}