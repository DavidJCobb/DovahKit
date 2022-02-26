#include "command_buffer.h"
#include "surface_renderer.h"

namespace vulkanDK {
   command_buffer::command_buffer(VkDevice d, VkCommandPool p) : owning_device(d), owning_pool(p) {
      this->_setup();
   }
   command_buffer::command_buffer(surface_renderer& sr) : command_buffer(sr.logical_device, sr.command_pools.persistent) {
      this->_setup();
   }
   command_buffer::~command_buffer() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      vkFreeCommandBuffers(this->owning_device, this->owning_pool, 1, &this->handle);
   }

   /*static*/ command_buffer command_buffer::create_transient(surface_renderer& sr) {
      return command_buffer(sr.logical_device, sr.command_pools.transient);
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
      return create_in_bulk(sr.logical_device, sr.command_pools.persistent, count);
   }

   VkResult command_buffer::reset(VkCommandBufferResetFlags flags) {
      return vkResetCommandBuffer(this->handle, flags);
   }
   VkResult command_buffer::top_level_begin(VkCommandBufferUsageFlags flags) {
      auto buffer_begin_info = VkCommandBufferBeginInfo{
         .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags            = 0,
         .pInheritanceInfo = nullptr,
      };
      return vkBeginCommandBuffer(this->handle, &buffer_begin_info);
   }
   VkResult command_buffer::finish() {
      return vkEndCommandBuffer(this->handle);
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