#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "helpers/vertex_index_list.h"
#include "buffer.h"
#include "vertex.h"
#include "scene_frame_item.h"

// geometry
#include "../helpers/vector3.h"

namespace vulkanDK {
   struct mesh_animation_state {
      bool  playing  = true;
      float duration = 4.0;
      float elapsed  = 0.0;
   };

   class rendered_mesh {
      protected:
         void _on_shader_parameter_change();
      public:
         rendered_mesh() {}
         ~rendered_mesh();

         rendered_mesh(rendered_mesh&&) noexcept;
         rendered_mesh& operator=(rendered_mesh&&) noexcept;

         struct shader_parameters { // pass to the shader via a storage buffer
            alignas(16) glm::mat4 transform;
            alignas(16) glm::vec3 specular_color    = { 0, 0, 0 };
            alignas( 4) float     specular_strength = 1.0;
            alignas( 4) float     specular_exponent = 32;
         };
         struct push_constant {
            int32_t object_index;
            int32_t texture_index;
         };
         
         struct {
            std::vector<vertex> vertices;
            vertex_index_list   indices;
            //
            struct {
               glm::vec3 center    = { 0.0, 0.0, 0.0 };
               float     radius_sq = 0.0F; // radius squared is faster for many calculations
            } bounding_sphere;
         } data;
         struct {
            buffer   buffer;
            uint32_t indices_at   = 0;
            uint32_t index_count  = 0;
            bool     wide_indices = false;
         } vertex_and_index_buffer;
         shader_parameters shader_params;
         int32_t texture_index = -1;
         //
         frame_dirty_state handled_frames; // for normal objects: frames that have had shader params synchronized. for pending-delete objects: frames that have been unhooked (when all are unhooked, we can delete the VIB)
         scene_frame_item_state life_state = scene_frame_item_state::empty;
         //
         mesh_animation_state* anim_state = nullptr; // owns

         inline bool active() const noexcept { return this->life_state == scene_frame_item_state::active; }
         inline bool empty() const noexcept { return this->life_state == scene_frame_item_state::empty; }
         inline bool pending_delete() const noexcept { return this->life_state == scene_frame_item_state::pending_delete; }
         
         inline const glm::mat4& transform() const noexcept { return this->shader_params.transform; }
         void set_transform(const glm::mat4&);

         // Setup functions:
         void recalc_bounding_sphere(); // does not count transforms, though member functions which use it should apply transforms
         //
         size_t total_size_for_setup() const;
         void sizes_for_setup(VkDeviceSize& v, VkDeviceSize& i, VkDeviceSize& total) const;
         void setup_vib_data_at(void*) const;

         // Caller should bind descriptor sets, send necessary push constants, etc., before calling this
         void draw_call(VkCommandBuffer);

         void mark_for_delete();
         void reset();

         // Object-local raycasts
         bool ray_intersects_bounding_sphere(const cobb::vector3<float>& ray_origin, cobb::vector3<float> ray_direction) const; // ray direction must be normalized
         bool ray_intersects_shape(const glm::vec3& ray_origin, glm::vec3 ray_direction, float& hit_distance) const; // ray direction must be normalized

         // World-local raycast
         bool ray_intersects(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& hit_distance) const;
   };
}