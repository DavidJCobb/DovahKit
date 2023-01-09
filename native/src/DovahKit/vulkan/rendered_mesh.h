#pragma once
#include <cstdint>
#include <type_traits>
#include <vector>
#include <glm/glm.hpp>
#include "./_vulkan.h"
#include "./helpers/vertex_index_list.h"
#include "./buffer.h"
#include "./loaded_texture_index.h"
#include "./vertex.h"

#include "./scene_entities/base.h"
#include "./scene_entities/owned_gpu_resource_sets.h"

// geometry
#include "../helpers/vector3.h"

namespace vulkanDK {
   class  command_buffer;
   class  raycast;
   struct raycast_hit_data;
   class  rendered_nif;

   namespace scene_entities {
      class owned_gpu_resource_upload_operation;
   }
}

namespace vulkanDK {
   struct mesh_animation_state {
      bool  playing  = true;
      float duration = 4.0;
      float elapsed  = 0.0;
   };

   class rendered_mesh : public scene_entities::base {
      public:
         static constexpr const char* name_single = "mesh";
         static constexpr const char* name_plural = "meshes";
         //
         static constexpr const bool owned_gpu_resources_are_coalesced = false;
         static constexpr const bool is_drawn = true;
      public:
         rendered_mesh() {}
         ~rendered_mesh();

         rendered_mesh(rendered_mesh&&) noexcept;
         rendered_mesh& operator=(rendered_mesh&&) noexcept;

         void mark_for_delete();
         void reset_for_recycle();
         void reset();

         #pragma region Flags masks
            struct cull_flag {
               enum type : uint32_t {
                  culled_by_application = 0x00000001,
               };
            };
            using cull_flags_t = std::underlying_type_t<cull_flag::type>;

            struct mesh_flag { // flags applied on the CPU, not within shaders
               enum type : uint32_t {
                  requires_oit          = 0x00000001,
                  double_sided          = 0x00000002,
                  cast_shadows          = 0x00000004,
                  is_decal              = 0x00000008,
                  culled_by_application = 0x00000010,
                  is_editor_marker      = 0x00000020, // is, or is inside of, any NiObjectNET with name "EditorMarker"
               };
               static constexpr uint32_t all_default_flags = cast_shadows;
            };
            using mesh_flags_t = std::underlying_type_t<mesh_flag::type>;
         #pragma endregion

         struct frame_drawing_data_type { // pass to the shader via a storage buffer
            alignas(16) glm::mat4 transform;
            //
            alignas(16) glm::vec3 specular_color    = { 0, 0, 0 };
            alignas( 4) float     specular_strength = 1.0;
            alignas( 4) float     specular_exponent = 32;
            //
            alignas(16) glm::vec3 bounding_sphere_center = { 0, 0, 0 };
            alignas( 4) float     bounding_sphere_radius = 0;
         };
         struct frame_culling_data_type {
            alignas(16) glm::mat4    transform;
            alignas(16) glm::vec3    bounding_sphere_center = {};
            alignas( 4) float        bounding_sphere_radius = 0;
            alignas( 4) cull_flags_t flags = 0;
         };
         struct push_constant {
            //
            // These three values are not persistent; this entire push constant 
            // is copied during the render process, and on the copy, these values 
            // are overwritten with data stored elsewhere on the entity.
            //
            alignas(4) int32_t  object_index;
            alignas(4) int32_t  texture_index;
            alignas(4) int32_t  texture_normal_index  = -1;
            //
            // Values below are "persistent" and configure the rendered mesh 
            // directly.
            //
            alignas(4) float    alpha_test_threshold  =  0;
            alignas(4) int32_t  alpha_test_operation  =  0; // GL_ALWAYS
            alignas(4) VkBool32 enable_alpha_blending = VK_FALSE; // bools in GLSL are uint32_ts in SPIR-V
            alignas(4) VkBool32 receive_shadows       = VK_TRUE;  // bools in GLSL are uint32_ts in SPIR-V
         };

         struct vertex_and_index_buffer {
            buffer   buffer;
            uint32_t indices_at   = 0;
            uint32_t index_count  = 0;
            bool     wide_indices = false;

            inline bool empty() const noexcept { return this->buffer.empty(); }
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
         } mesh_data;
         union texture_indices_union {
            texture_indices_union() : list({}) {}
            ~texture_indices_union() {}

            std::array<loaded_texture_index, 2> list;
            struct {
               loaded_texture_index diffuse;
               loaded_texture_index normals;
            };
         } texture_indices;

         scene_entities::owned_gpu_resource_sets<vertex_and_index_buffer> owned_gpu_resources;
         frame_drawing_data_type frame_drawing_data;
         push_constant           push_params;
         
         rendered_nif* owning_nif = nullptr;
         mesh_animation_state* anim_state = nullptr; // owns

         [[nodiscard]] frame_culling_data_type calculate_frame_culling_data() const;

         constexpr vertex_and_index_buffer& vib() noexcept { return this->owned_gpu_resources.current; }
         constexpr const vertex_and_index_buffer& vib() const noexcept { return this->owned_gpu_resources.current; }

         constexpr const glm::mat4& transform() const noexcept { return this->frame_drawing_data.transform; }
         void set_transform(const glm::mat4&);

         // Setup functions:
         void recalc_bounding_sphere(); // does not count transforms, though member functions which use it should apply transforms

         #pragma region Member functions for owned GPU resources (esp. for uploading)
         VkDeviceSize owned_gpu_resources_size() const noexcept;
         void upload_owned_gpu_resources(scene_entities::owned_gpu_resource_upload_operation&);
         #pragma endregion

         // Caller should bind descriptor sets, send necessary push constants, etc., before calling this
         void draw_call(VkCommandBuffer);
         [[nodiscard]] VkDrawIndexedIndirectCommand make_indirect_draw_command() const;

         // World-relative raycasts (uses the mesh's transform):
         bool ray_intersects_bounding_sphere(const cobb::vector3<float>& ray_origin, cobb::vector3<float> ray_direction) const;

         // Sets barycentric position, hit distance, and hit position.
         // Does not set surface normal. You should request that just from the final hit entity.
         // Ignores backfaces.
         raycast_hit_data do_raycast(const raycast&) const;

         // Returns world-relative surface normal, accounting for mesh's transform. Triangle index is NOT bounds-checked.
         glm::vec3 triangle_surface_normal(size_t triangle_index) const;
   };
}