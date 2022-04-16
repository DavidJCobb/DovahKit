#include "buffer.h"
#include <algorithm>
#include "command_buffer.h"
#include "exceptions.h"
#include "surface_renderer.h"

namespace vulkanDK {
   buffer::buffer(surface_renderer& d) : renderer(&d) {
   }
   buffer::~buffer() {
      this->teardown();
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
      auto alloc_info = VmaAllocationCreateInfo{
         .flags          = 0,
         .usage          = VMA_MEMORY_USAGE_UNKNOWN,
         .requiredFlags  = properties,
         .preferredFlags = 0,
         .memoryTypeBits = 0,
         .pool           = nullptr,
         .pUserData      = nullptr,
         .priority       = 0,
      };
      if (auto result = vmaCreateBuffer(owner.allocator, &buffer_info, &alloc_info, &out.handle, &out.memory, nullptr); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::buffer::create] Failed to create buffer handle.");
      }
      out.size = size;
      //
      return out;
   }

   void buffer::teardown() {
      if (this->handle != VK_NULL_HANDLE) {
         assert(this->renderer);
         vmaDestroyBuffer(this->renderer->allocator, this->handle, this->memory);
         this->handle = VK_NULL_HANDLE;
         this->memory = VK_NULL_HANDLE;
      } else {
         assert(this->memory == VK_NULL_HANDLE);
      }
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
      void* data;
      vmaMapMemory(this->renderer->allocator, this->memory, &data);
      if (offset) {
         return (void*)((std::intptr_t)data + offset);
      }
      return data;
   }
   void* buffer::map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags flags) {
      assert(this->renderer);
      void* data;
      vmaMapMemory(this->renderer->allocator, this->memory, &data);
      if (offset) {
         return (void*)((std::intptr_t)data + offset);
      }
      return data;
   }
   void buffer::unmap_memory(void* mapped) {
      assert(this->renderer);
      vmaUnmapMemory(this->renderer->allocator, this->memory);
   }

   VkResult buffer::flush_memory() {
      return vmaFlushAllocation(this->renderer->allocator, this->memory, 0, VK_WHOLE_SIZE);
   }
}