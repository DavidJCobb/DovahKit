#pragma once
#include <type_traits>
#include "_vulkan.h"
#include "_memory.h"
#include "_util.h"

#include "helpers/constexpr_optional_type.h"

namespace vulkanDK {
   class surface_renderer;

   class buffer : no_copy {
      friend class surface_renderer;
      protected:
         using memory_handle_type = std::conditional_t<use_vma_library, VmaAllocation, VkDeviceMemory>;

         memory_handle_type memory = VK_NULL_HANDLE;

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

         void copy_from(const buffer& source);
         void copy_from(const buffer& source, VkDeviceSize);

         void* map_memory(VkDeviceSize offset = 0, VkMemoryMapFlags = 0);
         void* map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags = 0);
         void unmap_memory(void* mapped);
   };
}