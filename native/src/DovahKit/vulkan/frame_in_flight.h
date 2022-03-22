#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"
#include "command_buffer.h"
#include "surface_renderer_descriptor_group.h"
#include "overlays/fps.h"
#include "overlays/world_axes.h"

namespace vulkanDK {
   class descriptor_set;
   class render_pass;
   class scene;
   class surface_renderer;

   class frame_in_flight : no_copy {
      protected:
         union command_buffer_set {
            std::array<command_buffer, 4> list;
            struct {
               command_buffer main_shadow;
               command_buffer main_shadow_placed;
               command_buffer main;
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
         frame_in_flight() {}
         ~frame_in_flight();

         frame_in_flight(frame_in_flight&&) noexcept = default;
         frame_in_flight& operator=(frame_in_flight&&) noexcept = default;
      
      protected:
         surface_renderer* owner = nullptr;
         size_t my_index = -1;
         //
      public:
         VkFence fence = VK_NULL_HANDLE; // synchronize the command buffer: you cannot "record" commands to it if it's still being "played" by the GPU, so wait on this fence before trying
         struct {
            VkSemaphore image_available = VK_NULL_HANDLE;
            VkSemaphore render_finished = VK_NULL_HANDLE;
         } semaphores;
         //
         descriptor_set_group descriptor_sets;
         command_buffer_set   command_buffers;
         struct {
            buffer uniform;     // per-scene  data which can be updated without having to re-record command buffers (scene_global_state)
            buffer object_data; // per-object data which can be updated without having to re-record command buffers (rendered_mesh::shader_parameters[])
            buffer light_data;  // per-light  data which can be updated without having to re-record command buffers (rendered_light::shader_parameters[])
         } shader_params;
         struct {
            overlays::fps        fps;
            overlays::world_axes world_axes;
         } overlays;
         //
         bool command_buffers_invalid = true;

         inline size_t index() const noexcept { return this->my_index; }

         void setup(surface_renderer&, size_t my_index);
         void setup_descriptor_sets();

         void pre_resize();
         void post_resize();

         void teardown_descriptor_sets();
         void teardown();

         void record_draw_commands();

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
         void _update_shader_lights_data_buffer();
         void _update_shader_object_data_buffer();
         void _update_shader_texture_descriptors();

         void _refill_command_buffers();
         void _refill_fps_overlay_command_buffer();
   };
}
