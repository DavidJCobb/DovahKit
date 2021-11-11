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

   class frame_in_flight : no_copy {
      public:
         frame_in_flight() {}
         ~frame_in_flight();

         frame_in_flight(frame_in_flight&&) noexcept = default;
         frame_in_flight& operator=(frame_in_flight&&) noexcept = default;
      
         surface_renderer* owner = nullptr;
         //
         VkFence fence = VK_NULL_HANDLE; // synchronize the command buffer: you cannot "record" commands to it if it's still being "played" by the GPU, so wait on this fence before trying
         struct {
            VkSemaphore image_available = VK_NULL_HANDLE;
            VkSemaphore render_finished = VK_NULL_HANDLE;
         } semaphores;

         void setup(surface_renderer&);

      protected:
         void _setup_semaphores();
   };
}
