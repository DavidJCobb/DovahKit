#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"
#include "command_buffer.h"

namespace vulkanDK {
   class context;
   class descriptor_set;
   class render_pass;

   class frame_render_pass {
      public:
         render_pass* pass = nullptr;
         std::vector<command_buffer> command_buffers;
         bool command_buffers_invalid = false;
   };

   class frame_in_flight : no_copy {
      public:
         frame_in_flight(context&);
         ~frame_in_flight();
      
         context* owner = nullptr;
         //
         VkFence fence;
         struct {
            VkSemaphore image_available;
            VkSemaphore render_finished;
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

         void teardown_for_resize();

      protected:
         void _setup_semaphores();
         void _setup_shader_parameter_buffers();
         void _setup_descriptor_sets();
         void _setup_command_buffers();
   };
}
