#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"
#include "command_buffer.h"

namespace vulkanDK {
   class descriptor_set;
   class render_pass;
   class scene;
   class surface_renderer;

   class frame_render_pass {
      public:
         render_pass* pass = nullptr;
         std::vector<command_buffer> command_buffers;
         bool command_buffers_invalid = false;
   };

   class frame_in_flight : no_copy {
      public:
         frame_in_flight() {}
         ~frame_in_flight();
      
         surface_renderer* owner = nullptr;
         size_t my_index = 0; // needed for scene updates
         //
         VkFence fence = VK_NULL_HANDLE;
         struct {
            VkSemaphore image_available = VK_NULL_HANDLE;
            VkSemaphore render_finished = VK_NULL_HANDLE;
         } semaphores;
         //
         std::vector<VkDescriptorSet>   descriptor_sets;
         std::vector<frame_render_pass> render_passes;
         struct {
            buffer uniform;
            buffer object_data;
         } shader_params;

         void draw(VkFramebuffer);

         void invalidate_all_command_buffers();

         void setup(surface_renderer&, size_t which_am_i);

      protected:
         void _setup_semaphores();
         void _setup_shader_parameter_buffers();
         void _setup_descriptor_sets();
         void _setup_command_buffers();
         
         // draw steps:
         scene& get_scene();
         void _update_shader_object_data_buffer();
         void _update_shader_texture_descriptors();
         void _refill_command_buffers(VkFramebuffer);
   };
}
