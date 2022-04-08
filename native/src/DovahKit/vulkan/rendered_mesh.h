#pragma once
#include <cstdint>
#include <type_traits>
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

         struct mesh_flag { // flags applied on the CPU, not within shaders
            enum type : uint32_t {
               requires_oit = 0x00000001,
               double_sided = 0x00000002,
               cast_shadows = 0x00000004,
               is_decal     = 0x00000008,
            };
            //
            static constexpr uint32_t all_default_flags = cast_shadows;
         };
         using mesh_flags_t = std::underlying_type_t<mesh_flag::type>;

         struct shader_parameters { // pass to the shader via a storage buffer
            alignas(16) glm::mat4 transform;
            //
            alignas(16) glm::vec3 specular_color    = { 0, 0, 0 };
            alignas( 4) float     specular_strength = 1.0;
            alignas( 4) float     specular_exponent = 32;
            //
            alignas( 4) float     bounding_sphere_radius = 0;
         };
         struct push_constant {
            alignas(4) int32_t  object_index;               // not meaningful on this object; ignored during the render process
            alignas(4) int32_t  texture_index;              // not meaningful on this object; ignored during the render process
            alignas(4) int32_t  texture_normal_index  = -1; // not meaningful on this object; ignored during the render process
            alignas(4) float    alpha_test_threshold  =  0;
            alignas(4) int32_t  alpha_test_operation  =  0; // GL_ALWAYS
            alignas(4) VkBool32 enable_alpha_blending = VK_FALSE; // bools in GLSL are uint32_ts in SPIR-V
            alignas(4) VkBool32 receive_shadows       = VK_TRUE;  // bools in GLSL are uint32_ts in SPIR-V
         };
         
         mesh_flags_t mesh_flags = mesh_flag::all_default_flags;
         struct {
            std::vector<vertex> vertices;
            vertex_index_list   indices;
            //
            struct {
               glm::vec3 min = { 0.0, 0.0, 0.0 };
               glm::vec3 max = { 0.0, 0.0, 0.0 };
            } bounding_box;
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
         push_constant     push_params;
         union {
            std::array<int32_t, 2> list = { -1, -1 };
            struct {
               int32_t diffuse;
               int32_t normals;
            };
         } texture_indices;
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
         VkDrawIndexedIndirectCommand make_indirect_draw_command() const;

         void mark_for_delete();
         void reset();

         // World-relative raycasts (uses the mesh's transform):
         bool ray_intersects_bounding_sphere(const cobb::vector3<float>& ray_origin, cobb::vector3<float> ray_direction) const;
         bool ray_intersects_shape(const glm::vec3& ray_origin, glm::vec3 ray_direction, float& hit_distance) const;
         bool ray_intersects(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& hit_distance) const;
   };
}