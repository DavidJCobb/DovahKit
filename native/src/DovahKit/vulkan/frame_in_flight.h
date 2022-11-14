#pragma once
#include <vector>
#include "helpers/class_map.h"
#include "./helpers/bundled_descriptor_buffer_write.h"
#include "_vulkan.h"
#include "_util.h"
#include "config/scene_limits.h"
#include "buffer.h"
#include "command_buffer.h"
#include "surface_renderer_descriptor_group.h"
#include "overlays/fps.h"
#include "overlays/world_axes.h"
#include "scene_entities/all_types.h"

namespace vulkanDK {
   class descriptor_set;
   class render_pass;
   class rendered_mesh;
   class scene;
   class surface_renderer;

   union frame_in_flight_fence_set {
      //
      // These fences allow us to synchronize the CPU with command buffer execution on the 
      // GPU: you cannot "record" commands to a command buffer if it's still being "played" 
      // by the GPU, so wait on the relevant fences before trying.
      //
      std::array<VkFence, 2> list = { VK_NULL_HANDLE, VK_NULL_HANDLE };
      struct {
         VkFence compute;
         VkFence graphics;
      };

      inline bool empty() const {
         for (auto& item : list)
            if (item != VK_NULL_HANDLE)
               return false;
         return true;
      }
      inline VkResult wait_on_all(VkDevice device) const {
         return vkWaitForFences(device, list.size(), list.data(), VK_TRUE, UINT64_MAX);
      }
   };

   class frame_in_flight : no_copy {
      protected:
         union command_buffer_set {
            std::array<command_buffer, 5> list;
            struct {
               command_buffer main_shadow;
               command_buffer main_shadow_placed;
               command_buffer main;
               command_buffer bounds;
               command_buffer fps;
            };

            command_buffer_set() : list({}) {};
            ~command_buffer_set() {
               for (auto& item : list)
                  item.~command_buffer();
            }

            command_buffer_set(command_buffer_set&& v) {
               for (size_t i = 0; i < this->list.size(); ++i)
                  std::swap(this->list[i], v.list[i]);
            }
            command_buffer_set& operator=(command_buffer_set&& v) {
               for (size_t i = 0; i < this->list.size(); ++i)
                  std::swap(this->list[i], v.list[i]);
               return *this;
            }

            void setup(surface_renderer& sr) {
               for (auto& item : list)
                  item = command_buffer(sr);
            }
         };
         static_assert(sizeof(command_buffer_set) == sizeof(command_buffer) * std::tuple_size_v<decltype(command_buffer_set::list)>);

      public:
         struct indirect_draw_buffers {
            using mesh_index_list = std::array<int32_t, config::max_rendered_meshes>;
            static constexpr size_t mesh_index_list_size = sizeof(mesh_index_list::value_type) * config::max_rendered_meshes;

            buffer params;
            struct {
               buffer gpu;
               std::unique_ptr<mesh_index_list> host;
            } mesh_indices;
            struct {
               VkBufferMemoryBarrier to_compute;
               VkBufferMemoryBarrier to_graphics;
            } barriers;

            void setup(surface_renderer&, const std::string& debug_name = "");

            //
            // VkBuffers are only usable from one VkQueue (their "owner") at a time, unless the buffers 
            // were created with VK_SHARING_MODE_CONCURRENT (which is slower). If you need to use a 
            // buffer from multiple queues, without destroying its contents, then you must perform a 
            // Queue Family Ownership Transfer. This entails submitting memory barrier commands on both 
            // the source queue and the destination queue.
            // 
            // These calls should be made from outside of a render pass. Using these commands within a 
            // render pass requires that the relevant subpass have a dependency on itself, but that's 
            // not possible for a compute-to-graphics barrier (or vice versa) because compute shaders 
            // themselves cannot exist within a render pass.
            //
            void transfer_queue_ownership_to_compute(VkCommandBuffer) const;
            void transfer_queue_ownership_to_graphics(VkCommandBuffer) const;
         };

      public:
         frame_in_flight();
         ~frame_in_flight();
      
      protected:
         surface_renderer* owner = nullptr;
         size_t my_index = -1;
         buffer idb_mesh_index_staging;
         buffer idb_params_staging;
         //
      public:
         frame_in_flight_fence_set fences;
         struct {
            VkSemaphore image_available   = VK_NULL_HANDLE; // swap chain image acquired
            VkSemaphore compute_finished  = VK_NULL_HANDLE; // compute commands done execution
            VkSemaphore graphics_finished = VK_NULL_HANDLE; // graphics commands done execution
            VkSemaphore render_finished   = VK_NULL_HANDLE; // last submission is done execution
         } semaphores;
         //
         descriptor_set_group descriptor_sets;
         command_buffer_set   graphics_commands;
         struct {
            command_buffer frustum_cull_main;
            command_buffer frustum_cull_sun;
            command_buffer shadow_caster_cull;
         } compute_commands;
         struct {
            indirect_draw_buffers main;
            indirect_draw_buffers main_oit;
            indirect_draw_buffers sun_shadows;
            std::array<indirect_draw_buffers, config::max_active_shadow_casters> shadow_casters;
         } indirect_draw_commands;
         struct {
            cobb::class_map_from_class_array<buffer, scene_entities::all_types_with_fixed_length_coalesced_vibs> fixed_length;
         } coalesced_vibs;
         struct {
            //
            // Each frustum is defined by four vec4s which are the inward-facing surface normals of 
            // the left, right, top, and bottom faces of the frustum.
            //
            buffer main;
            buffer sun;
         } shader_frustums;
         struct {
            //
            // For compute shaders:
            //
            buffer active_caster_positions; // position vectors for each active shadow caster, for the compute shader
            cobb::class_map_from_class_array<buffer, scene_entities::all_types_with_frame_culling_data> scene_entity_frame_culling_data;
            //
            // For graphics:
            //
            buffer scene_data;        // scene_global_state: per-scene data which can be updated without having to re-record command buffers
            buffer gizmo_data;        // scene_gizmo_state
            buffer light_shadow_data; // glm::mat4[] array: six view matrices per shadow caster
            cobb::class_map_from_class_array<buffer, scene_entities::all_types_with_frame_drawing_data> scene_entity_frame_drawing_data;
         } shader_params;
         struct {
            overlays::fps        fps;
            overlays::world_axes world_axes;
         } overlays;
         //
         bool command_buffers_invalid = true;
         struct {
            bool compute_culling_ran_once = false;
         } debug;

         inline size_t index() const noexcept { return this->my_index; }

         void setup(surface_renderer&, size_t my_index);
         void setup_descriptor_sets();

         void pre_resize();
         void post_resize();

         void teardown_descriptor_sets();
         void teardown();

      protected:
         struct {
            bool recorded_compute_cull_commands = false;
            cobb::class_map_from_class_array<bool, scene_entities::all_types_that_are_drawn> scene_entity_draws_changed;
            cobb::class_map_from_class_array<bool, scene_entities::all_types_with_fixed_length_coalesced_vibs> scene_entity_coalesced_vib_resize_needed;
            bool must_re_record_graphics = true;
            bool must_re_record_ui       = true;
            //
            bool must_update_gizmo_state = true;
         } state;
      public:
         void prepare_for_render();
         void prepare_indirect_draws();
         void record_compute_cull_commands();
         void record_graphics_commands();
         protected:
            void _record_scene_draw_commands();
               void _record_bounds_draw_commands();
               void _record_gizmo_draw_commands();
            void _record_ui_draw_commands();
      public:
         void submit_compute_cull_commands();
         void submit_graphics_commands(const std::vector<VkCommandBuffer>& append_command_buffers = {});

         template<typename Entity> void on_scene_entity_added_or_removed() {
            if constexpr (scene_entities::all_types_that_are_drawn::contains_type<Entity>) {
               this->state.scene_entity_draws_changed.value_for<Entity>() = true;
            }
            if constexpr (scene_entities::all_types_with_variable_length_coalesced_vibs::contains_type<Entity>) {
               this->state.scene_entity_coalesced_vib_resize_needed.value_for<Entity>() = true;
            }
         }
         template<typename Entity> requires (scene_entities::all_types_with_owned_gpu_resources::contains_type<Entity>)
         void on_scene_entity_owned_gpu_resource_upload_complete() {
            if constexpr (scene_entities::all_types_that_are_drawn::contains_type<Entity>) {
               this->state.scene_entity_draws_changed.value_for<Entity>() = true;
            }
         }
         void on_gizmo_mode_changed();
         void on_gizmo_state_changed() {
            this->state.must_update_gizmo_state = true;
         }
         void invalidate_all_command_buffers();

      protected:
         void _setup_semaphores();
         void _setup_shader_parameter_buffers();
         void _setup_descriptor_sets();
         void _setup_command_buffers();

         // draw steps:
         void _hook_to_frame(frame_in_flight&);
         //
         scene& get_scene();
         void _update_shader_global_scene_state();
         void _update_shader_global_gizmo_state();
         void _update_shader_texture_descriptors();

         // helper functions for the templated member functions defined in the INL file, to avoid 
         // circular-dependency issues with calling `surface_renderer` member functions:
         [[nodiscard]] VkDevice _logical_device_handle() const;
         [[nodiscard]] buffer _create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);
         [[nodiscard]] buffer _create_staging_buffer(VkDeviceSize size);
         void _set_debug_object_name(buffer&, const char*);

         template<typename Entity> void _allocate_scene_entity_coalesced_vib();
         template<typename Entity> void _update_scene_entity_coalesced_vib();

         template<typename Entity> void _update_drawn_scene_entity_frame_data();
   };
}

#include "./frame_in_flight.inl"