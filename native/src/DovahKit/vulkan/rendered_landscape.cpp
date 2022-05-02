#include "rendered_landscape.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/LandTexture.h"
#include "dovah/forms/TextureSet.h"

#include <QDebug>

namespace vulkanDK {
   static_assert(rendered_landscape::verts_per_mesh == rendered_landscape::loaded_form::total_vertex_count);

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
      auto& vl = this->vertices;
      for (size_t i = 0; i < verts_per_mesh; ++i) {
         const auto& color = land.heightmap.colors.by_flat_index(i);
         if (land.land_flags & loaded_form::land_flag::has_colors) {
            vl[i].color = glm::vec3{ (float)color.r, (float)color.g, (float)color.b } / 255.0F;
         } else {
            vl[i].color = glm::fvec3{ 1, 1, 1 };
         }
         //
         vl[i].height = land.heightmap.heights.by_flat_index(i);
         for (size_t j = 0; j < vl[i].blends.size(); ++j)
            vl[i].blends[j] = 0;
      }
      //
      constexpr bool dont_even_bother_dealing_with_the_quads = true;
      //
qDebug("[rendered_landscape::import_vertex_data_from_form] Generating landscape vertex data...");
      for (size_t q = 0; q < 4; ++q) {
qDebug(" - Quad %d", q);
         for (auto& blend : land.alpha_layers_by_quad[q]) {
            auto layer = blend.layer;
qDebug("    - Layer %d", layer);
            if (layer < 0 || layer >= rendered_landscape::max_usable_layers_per_quad)
               continue;
qDebug("      Proceeding...");
            //
            auto& alphas = blend.opacities;
{
   qDebug("       - Dumping blends:");
   for (size_t y = 0; y < verts_per_side; ++y) {
      QString line = "         ";
      for (size_t x = 0; x < verts_per_side; ++x) {
         line += QString::number(alphas.item(x, y), 'f', 2) + ' ';
      }
      qDebug(qUtf8Printable(line));
   }
}
            if constexpr (dont_even_bother_dealing_with_the_quads) {
               //
               // Blends are stored as four 17x17 quadrants with one vertex of overlap, 
               // covering the full 33x33 cell. Mapping quadrant blends to whole-cell 
               // coordinates SHOULD be easy, but it just isn't working no matter what 
               // I try.
               //
               for (size_t y = 0; y < verts_per_side; ++y) {
                  for (size_t x = 0; x < verts_per_side; ++x) {
                     auto f = alphas.item(x, y);
                     if (f <= 0)
                        continue;
                     auto i = (y * verts_per_side) + x;
                     vl[i].blends[layer] = f;
                  }
               }
            } else {
               using alpha_grid = std::remove_reference_t<decltype(alphas)>;
               //
               uint8_t x_min =  0;
               uint8_t x_max = 17;
               uint8_t y_min =  0;
               uint8_t y_max = 17;
               land.quad_coords_to_cell_coords(q, x_min, y_min);
               land.quad_coords_to_cell_coords(q, x_max, y_max);
               //
               for (size_t y = y_min; y < y_max; ++y) {
                  for (size_t x = x_min; x < x_max; ++x) {
                     auto i = (y * verts_per_side) + x;
                     vl[i].blends[layer] = alphas.item(x, y);
                  }
               }
            }
         }
      }
{
   qDebug("       - Dumping final data:");
   for (size_t b = 0; b < max_usable_layers_per_quad; ++b) {
      qDebug("          - Blend %d:", b);
      for (size_t y = 0; y < verts_per_side; ++y) {
         QString line = "            ";
         for (size_t x = 0; x < verts_per_side; ++x) {
            auto& v = vl[y * verts_per_side + x];
            line += QString::number(v.blends[b], 'f', 2) + ' ';
         }
         qDebug(qUtf8Printable(line));
      }
   }
}
qDebug(" - Data dumped.");
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