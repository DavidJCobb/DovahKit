#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class surface_renderer;

   class command_buffer : no_copy {
      protected:
         VkDevice      owning_device = VK_NULL_HANDLE;
         VkCommandPool owning_pool   = VK_NULL_HANDLE;
      public:
         command_buffer() {}
         command_buffer(VkDevice, VkCommandPool);
         command_buffer(surface_renderer&);
         ~command_buffer();

         command_buffer(command_buffer&&) noexcept;
         command_buffer& operator=(command_buffer&&) noexcept;

         VkCommandBuffer handle = VK_NULL_HANDLE;

         static std::vector<command_buffer> create_in_bulk(VkDevice, VkCommandPool, size_t count);
         static std::vector<command_buffer> create_in_bulk(surface_renderer&, size_t count);

      protected:
         void _setup();
   };
}