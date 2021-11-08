#pragma once
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class context;
   class frame_in_flight;
   class render_pass;

   class command_buffer : no_copy {
      public:
         command_buffer() {}
         command_buffer(context&);
         ~command_buffer();

         context* owner = nullptr;
         VkCommandBuffer handle = VK_NULL_HANDLE;
   };
}