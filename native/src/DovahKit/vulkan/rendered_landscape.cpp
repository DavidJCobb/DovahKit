#include "rendered_landscape.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/LandTexture.h"
#include "dovah/forms/TextureSet.h"

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
         const auto& color = land.heightmap.colors.list[i];
         vl[i].color = glm::vec3{ (float)color.r, (float)color.g, (float)color.b } / 255.0F;
         //
         vl[i].height = land.heightmap.heights.list[i];
         for (size_t j = 0; j < vl[i].blends.size(); ++j)
            vl[i].blends[j] = 0;
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