#include "buffer.h"
#include <algorithm>
#include "command_buffer.h"
#include "surface_renderer.h"

namespace vulkanDK {
   namespace {
      //
      // if-constexpr only prevents the false branch from being parsed and compiled 
      // when used in templates... ugh...
      //
      template<typename M> void _teardown(buffer& b, M memory) {
         if constexpr (use_vma_library) {
            vmaDestroyBuffer(b.renderer->allocator, b.handle, memory);
         } else {
            vkDestroyBuffer(b.renderer->logical_device, b.handle, nullptr);
            if (memory != VK_NULL_HANDLE) {
               vkFreeMemory(b.renderer->logical_device, memory, nullptr);
            }
         }
      }

      template<typename M> void _allocate(buffer& b, M& memory, VkBufferCreateInfo& buffer_info, VkMemoryPropertyFlags properties) {
         if constexpr (use_vma_library) {
            auto alloc_info = VmaAllocationCreateInfo{
               .usage         = VMA_MEMORY_USAGE_UNKNOWN,
               .requiredFlags = properties,
            };
            vmaCreateBuffer(b.renderer->allocator, &buffer_info, &alloc_info, &b.handle, &memory, nullptr);
         } else if constexpr (!use_vma_library) {
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(this->logical_device, b.handle, &memRequirements);
            //
            // In a real-world application, you wouldn't use vkAllocateMemory for each individual object you wish 
            // to render, because there's actually a limit on the number of allocations you can make irrespective 
            // of their total size. Even on high-end hardware, that limit may be in the low thousands, the Vulkan 
            // tutorial gives 4096 as a plausible limit for  hardware like an NVIDIA GTX 1080. What you'd want to 
            // do instead, then, is allocate memory in larger blocks and then manually divide those blocks up for 
            // different objects -- similar to what you'd do when making a block allocator.
            //
            auto alloc_info = VkMemoryAllocateInfo{
               .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .allocationSize = memRequirements.size,
               .memoryTypeIndex = b.renderer->device_info->find_memory_type(memRequirements.memoryTypeBits, properties),
            };
            if (vkAllocateMemory(b.renderer->logical_device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
               throw std::runtime_error("[vulkanDK::buffer::create] Failed to allocate buffer memory.");
            }
            vkBindBufferMemory(b.renderer->logical_device, b.handle, memory, 0);
         }
      }

      template<typename M> void* _map(buffer& b, M memory, VkMemoryMapFlags flags, VkDeviceSize offset) {
         void* data;
         if constexpr (use_vma_library) {
            vmaMapMemory(b.renderer->allocator, memory, &data);
         } else {
            vkMapMemory(b.renderer->logical_device, memory, offset, b.size - offset, flags, &data);
         }
         return data;
      }
      template<typename M> void* _map(buffer& b, M memory, VkMemoryMapFlags flags, VkDeviceSize offset, VkDeviceSize length) {
         void* data;
         if constexpr (use_vma_library) {
            vmaMapMemory(b.renderer->allocator, memory, &data);
         } else {
            vkMapMemory(b.renderer->logical_device, memory, offset, length, flags, &data);
         }
         return data;
      }
      template<typename M> void _unmap(buffer& b, M memory) {
         if constexpr (use_vma_library) {
            vmaUnmapMemory(b.renderer->allocator, memory);
         } else {
            vkUnmapMemory(b.renderer->logical_device, memory);
         }
      }
   }


   buffer::buffer(surface_renderer& d) : renderer(&d) {
   }
   buffer::~buffer() {
      if (this->handle != VK_NULL_HANDLE) {
         assert(this->renderer);
         _teardown(*this, this->memory);
      } else {
         assert(this->memory == VK_NULL_HANDLE);
      }
   }

   buffer::buffer(buffer&& o) noexcept {
      std::swap(this->renderer, o.renderer);
      std::swap(this->memory,   o.memory);
      std::swap(this->handle,   o.handle);
      std::swap(this->size,     o.size);
   }
   buffer& buffer::operator=(buffer&& o) noexcept {
      std::swap(this->renderer, o.renderer);
      std::swap(this->memory,   o.memory);
      std::swap(this->handle,   o.handle);
      std::swap(this->size,     o.size);
      return *this;
   }

   /*static*/ buffer buffer::create(surface_renderer& owner, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
      buffer out = buffer(owner);
      //
      auto buffer_info = VkBufferCreateInfo{
         .sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size         = size,
         .usage        = usage,
         .sharingMode  = VK_SHARING_MODE_EXCLUSIVE,
      };
      if (vkCreateBuffer(owner.logical_device, &buffer_info, nullptr, &out.handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::buffer::create] Failed to create buffer handle.");
      }
      _allocate(out, out.memory, buffer_info, properties);
      out.size = size;
      //
      return out;
   }

   void buffer::copy_from(const buffer& source) {
      this->renderer->do_single_commands([this, &source](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = source.size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }
   void buffer::copy_from(const buffer& source, VkDeviceSize size) {
      this->renderer->do_single_commands([this, &source, size](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }

   void* buffer::map_memory(VkDeviceSize offset, VkMemoryMapFlags flags) {
      assert(this->renderer);
      return _map(*this, this->memory, flags, offset);
   }
   void* buffer::map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags flags) {
      assert(this->renderer);
      return _map(*this, this->memory, flags, offset, length);
   }
   void buffer::unmap_memory(void* mapped) {
      assert(this->renderer);
      _unmap(*this, this->memory);
   }
}