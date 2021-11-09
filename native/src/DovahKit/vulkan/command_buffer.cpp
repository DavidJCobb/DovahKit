#include "command_buffer.h"
#include "logical_device.h"
#include "surface_renderer.h"

namespace vulkanDK {
   command_buffer::command_buffer(VkDevice d, VkCommandPool p) : owning_device(d), owning_pool(p) {
      this->_setup();
   }
   command_buffer::command_buffer(surface_renderer& sr) : owning_device(sr.device.handle), owning_pool(sr.command_pool) {
      this->_setup();
   }
   command_buffer::~command_buffer() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      vkFreeCommandBuffers(this->owning_device, this->owning_pool, 1, &this->handle);
   }

   command_buffer::command_buffer(command_buffer&& o) noexcept {
      std::swap(this->owning_device, o.owning_device);
      std::swap(this->owning_pool,   o.owning_pool);
      std::swap(this->handle,        o.handle);
   }
   command_buffer& command_buffer::operator=(command_buffer&& o) noexcept {
      std::swap(this->owning_device, o.owning_device);
      std::swap(this->owning_pool,   o.owning_pool);
      std::swap(this->handle,        o.handle);
      return *this;
   }
   
   /*static*/ std::vector<command_buffer> command_buffer::create_in_bulk(VkDevice d, VkCommandPool p, size_t count) {
      std::vector<VkCommandBuffer> handles(count);
      auto alloc_info = VkCommandBufferAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool        = p,
         .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = (uint32_t)handles.size(),
      };
      vkAllocateCommandBuffers(d, &alloc_info, handles.data());
      //
      std::vector<command_buffer> out(count);
      for (size_t i = 0; i < count; ++i) {
         auto& item = out[i];
         item.handle        = handles[i];
         item.owning_device = d;
         item.owning_pool   = p;
      }
      return out;
   }
   /*static*/ std::vector<command_buffer> command_buffer::create_in_bulk(surface_renderer& sr, size_t count) {
      return create_in_bulk(sr.device.handle, sr.command_pool, count);
   }

   void command_buffer::_setup() {
      auto alloc_info = VkCommandBufferAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool        = this->owning_pool,
         .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1,
      };
      vkAllocateCommandBuffers(this->owning_device, &alloc_info, &this->handle);
   }
}