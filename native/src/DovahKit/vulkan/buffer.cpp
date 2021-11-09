#include "buffer.h"
#include <algorithm>
#include "command_buffer.h"
#include "surface_renderer.h"
#include "logical_device.h"

namespace vulkanDK {
   buffer::buffer(logical_device& d) : device(&d) {
   }
   buffer::~buffer() {
      if (this->handle != VK_NULL_HANDLE) {
         assert(this->device);
         vkDestroyBuffer(this->device->handle, this->handle, nullptr);
         if (this->memory != VK_NULL_HANDLE) {
            vkFreeMemory(this->device->handle, this->memory, nullptr);
         }
      } else {
         assert(this->memory == VK_NULL_HANDLE);
      }
   }

   buffer::buffer(buffer&& o) noexcept {
      std::swap(this->device, o.device);
      std::swap(this->memory, o.memory);
      std::swap(this->handle, o.handle);
   }
   buffer& buffer::operator=(buffer&& o) noexcept {
      std::swap(this->device, o.device);
      std::swap(this->memory, o.memory);
      std::swap(this->handle, o.handle);
      return *this;
   }

   void buffer::copy_from(surface_renderer& c, const buffer& source) {
      c.do_single_commands([this, &source](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .size = this->size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }
   void buffer::copy_from(surface_renderer& c, const buffer& source, VkDeviceSize size) {
      c.do_single_commands([this, &source, size](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .size = size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }

   void* buffer::map_memory(VkDeviceSize offset, VkMemoryMapFlags flags) {
      assert(this->device);
      void* data;
      vkMapMemory(this->device->handle, this->memory, offset, this->size - offset, flags, &data);
      return data;
   }
   void* buffer::map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags flags) {
      assert(this->device);
      void* data;
      vkMapMemory(this->device->handle, this->memory, offset, length, flags, &data);
      return data;
   }
   void buffer::unmap_memory(void* mapped) {
      assert(this->device);
      vkUnmapMemory(this->device->handle, this->memory);
   }
}