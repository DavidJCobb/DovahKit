#include "command_buffer.h"
#include "context.h"

namespace vulkanDK {
   command_buffer::command_buffer(context& c) : owner(&c) {
      auto alloc_info = VkCommandBufferAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool        = c.command_pool,
         .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1,
      };
      vkAllocateCommandBuffers(c.logical_device(), &alloc_info, &this->handle);
   }
   command_buffer::~command_buffer() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      assert(this->owner);
      vkFreeCommandBuffers(this->owner->logical_device(), this->owner->command_pool, 1, &this->handle);
   }
}