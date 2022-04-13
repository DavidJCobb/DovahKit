#pragma once
#include <type_traits>
#include "_vulkan.h"
#include "_memory.h"
#include "_util.h"

namespace vulkanDK {
   class surface_renderer;

   class buffer : no_copy {
      friend class surface_renderer;
      protected:
         VmaAllocation memory = VK_NULL_HANDLE;

      public:
         buffer() {}
         buffer(surface_renderer&);
         ~buffer();

         buffer(buffer&&) noexcept;
         buffer& operator=(buffer&&) noexcept;
      
         surface_renderer* renderer = nullptr;
         VkBuffer          handle   = VK_NULL_HANDLE;
         VkDeviceSize      size     = 0;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }

         static buffer create(surface_renderer&, VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);
         
         void teardown();

         void copy_from(const buffer& source);
         void copy_from(const buffer& source, VkDeviceSize);

         void* map_memory(VkDeviceSize offset = 0, VkMemoryMapFlags = 0);
         void* map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags = 0);
         void unmap_memory(void* mapped);

         VkResult flush_memory(); // only needed for device-cached rather than device-coherent memory
   };
}