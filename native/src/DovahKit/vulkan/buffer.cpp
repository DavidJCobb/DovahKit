#include "buffer.h"
#include <algorithm>
#include "command_buffer.h"
#include "context.h"
#include "device.h"

namespace vulkanDK {
   buffer::buffer(device& d) : owner(&d) {
   }
   buffer::~buffer() {
      if (this->handle != VK_NULL_HANDLE) {
         assert(this->owner);
         vkDestroyBuffer(this->owner->logical, this->handle, nullptr);
         if (this->memory != VK_NULL_HANDLE) {
            vkFreeMemory(this->owner->logical, this->memory, nullptr);
         }
      } else {
         assert(this->memory == VK_NULL_HANDLE);
      }
   }

   buffer::buffer(buffer&& o) noexcept {
      std::swap(this->owner,  o.owner);
      std::swap(this->memory, o.memory);
      std::swap(this->handle, o.handle);
   }
   buffer& buffer::operator=(buffer&& o) noexcept {
      std::swap(this->owner,  o.owner);
      std::swap(this->memory, o.memory);
      std::swap(this->handle, o.handle);
      return *this;
   }

   void buffer::copy_from(context& c, const buffer& source) {
      c.do_single_commands([this, &source](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .size = this->size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }
   void buffer::copy_from(context& c, const buffer& source, VkDeviceSize size) {
      c.do_single_commands([this, &source, size](command_buffer& scratch) {
         auto copy_region = VkBufferCopy{
            .size = size,
         };
         vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
      });
   }
}