#include "unmanaged_buffer.h"
#include <algorithm>
#include "vulkan/command_buffer.h"
#include "vulkan/surface_renderer.h"

namespace vulkanDK {
   unmanaged_buffer::unmanaged_buffer(surface_renderer& d) : renderer(&d) {
   }
   unmanaged_buffer::~unmanaged_buffer() {
      if (this->handle != VK_NULL_HANDLE) {
         assert(this->renderer);
         vkDestroyBuffer(this->renderer->logical_device, this->handle, nullptr);
         if (this->memory != VK_NULL_HANDLE) {
            vkFreeMemory(this->renderer->logical_device, this->memory, nullptr);
         }
         this->handle = VK_NULL_HANDLE;
         this->memory = VK_NULL_HANDLE;
      } else {
         assert(this->memory == VK_NULL_HANDLE);
      }
   }

   unmanaged_buffer::unmanaged_buffer(unmanaged_buffer&& o) noexcept {
      std::swap(this->renderer, o.renderer);
      std::swap(this->memory,   o.memory);
      std::swap(this->handle,   o.handle);
      std::swap(this->size,     o.size);
   }
   unmanaged_buffer& unmanaged_buffer::operator=(unmanaged_buffer&& o) noexcept {
      std::swap(this->renderer, o.renderer);
      std::swap(this->memory,   o.memory);
      std::swap(this->handle,   o.handle);
      std::swap(this->size,     o.size);
      return *this;
   }

   /*static*/ unmanaged_buffer unmanaged_buffer::create(surface_renderer& owner, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
      unmanaged_buffer out = unmanaged_buffer(owner);
      //
      auto buffer_info = VkBufferCreateInfo{
         .sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size         = size,
         .usage        = usage,
         .sharingMode  = VK_SHARING_MODE_EXCLUSIVE,
      };
      if (vkCreateBuffer(owner.logical_device, &buffer_info, nullptr, &out.handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::unmanaged_buffer::create] Failed to create unmanaged_buffer handle.");
      }
      //
      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(owner.logical_device, out.handle, &memRequirements);
      //
      // In a real-world application, you wouldn't use vkAllocateMemory for each individual object you wish 
      // to render, because there's actually a limit on the number of allocations you can make irrespective 
      // of their total size. Even on high-end hardware, that limit may be in the low thousands, the Vulkan 
      // tutorial gives 4096 as a plausible limit for  hardware like an NVIDIA GTX 1080. What you'd want to 
      // do instead, then, is allocate memory in larger blocks and then manually divide those blocks up for 
      // different objects -- similar to what you'd do when making a block allocator.
      //
      auto alloc_info = VkMemoryAllocateInfo{
         .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize  = memRequirements.size,
         .memoryTypeIndex = owner.device_info->find_memory_type(memRequirements.memoryTypeBits, properties),
      };
      if (vkAllocateMemory(owner.logical_device, &alloc_info, nullptr, &out.memory) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::unmanaged_buffer::create] Failed to allocate buffer memory.");
      }
      vkBindBufferMemory(owner.logical_device, out.handle, out.memory, 0);
      //
      out.size = size;
      //
      return out;
   }

   void unmanaged_buffer::copy_from(const unmanaged_buffer& source) {
      this->renderer->do_single_commands([this, &source](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = source.size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }
   void unmanaged_buffer::copy_from(const unmanaged_buffer& source, VkDeviceSize size) {
      this->renderer->do_single_commands([this, &source, size](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }

   void* unmanaged_buffer::map_memory(VkDeviceSize offset, VkMemoryMapFlags flags) {
      assert(this->renderer);
      void* data;
      vkMapMemory(this->renderer->logical_device, memory, offset, this->size - offset, flags, &data);
      return data;
   }
   void* unmanaged_buffer::map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags flags) {
      assert(this->renderer);
      void* data;
      vkMapMemory(this->renderer->logical_device, memory, offset, length, flags, &data);
      return data;
   }
   void unmanaged_buffer::unmap_memory(void* mapped) {
      assert(this->renderer);
      vkUnmapMemory(this->renderer->logical_device, this->memory);
   }
}