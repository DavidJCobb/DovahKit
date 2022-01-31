#pragma once
#include <type_traits>
#include "vulkan/_vulkan.h"
#include "vulkan/_util.h"

namespace vulkanDK {
   class surface_renderer;

   class unmanaged_buffer : no_copy {
      friend class surface_renderer;
      protected:
         VkDeviceMemory memory = VK_NULL_HANDLE;

      public:
         unmanaged_buffer() {}
         unmanaged_buffer(surface_renderer&);
         ~unmanaged_buffer();

         unmanaged_buffer(unmanaged_buffer&&) noexcept;
         unmanaged_buffer& operator=(unmanaged_buffer&&) noexcept;
      
         surface_renderer* renderer = nullptr;
         VkBuffer          handle   = VK_NULL_HANDLE;
         VkDeviceSize      size     = 0;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }

         static unmanaged_buffer create(surface_renderer&, VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);

         void copy_from(const unmanaged_buffer& source);
         void copy_from(const unmanaged_buffer& source, VkDeviceSize);

         void* map_memory(VkDeviceSize offset = 0, VkMemoryMapFlags = 0);
         void* map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags = 0);
         void unmap_memory(void* mapped);
   };
}