#pragma once
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class logical_device;
   class surface_renderer;

   class buffer : no_copy {
      friend class logical_device;
      protected:
         VkDeviceMemory memory = VK_NULL_HANDLE; // temporary; replace with heap
      public:
         buffer() {}
         buffer(logical_device&);
         ~buffer();

         buffer(buffer&&) noexcept;
         buffer& operator=(buffer&&) noexcept;
      
         logical_device* device = nullptr;
         VkBuffer        handle = VK_NULL_HANDLE;
         VkDeviceSize    size   = 0;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }

         void copy_from(surface_renderer&, const buffer& source);
         void copy_from(surface_renderer&, const buffer& source, VkDeviceSize);

         void* map_memory(VkDeviceSize offset = 0, VkMemoryMapFlags = 0);
         void* map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags = 0);
         void unmap_memory(void* mapped);
   };
}