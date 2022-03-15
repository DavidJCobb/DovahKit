#include "command_buffer.h"
#include "material.h"
#include "render_pass.h"
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

   void command_buffer::_begin_render_pass(
      VkFramebuffer framebuffer,
      const VkRect2D render_area,
      const render_pass& pass,
      const VkClearValue* clear_values,
      size_t clear_value_count,
      VkSubpassContents contents
   ) {
      auto pass_begin_info = VkRenderPassBeginInfo{
         .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass      = pass.handle,
         .framebuffer     = framebuffer,
         .renderArea      = render_area,
         .clearValueCount = (uint32_t)clear_value_count,
         .pClearValues    = clear_values,
      };
      vkCmdBeginRenderPass(this->handle, &pass_begin_info, contents);
   }

   void command_buffer::_bind_material_and_descriptors(const material& mat, VkPipelineBindPoint bind_point, uint32_t bind_to, const VkDescriptorSet* sets, size_t ds_count, const uint32_t* dynamic_offsets, size_t do_count) {
      vkCmdBindDescriptorSets(this->handle, bind_point, mat.pipeline.layout, bind_to, (uint32_t)ds_count, sets, (uint32_t)do_count, dynamic_offsets);
      vkCmdBindPipeline(this->handle, bind_point, mat.pipeline.handle);
   }

   void command_buffer::_set_pipeline_push_constant(const material& m, VkShaderStageFlags flags, size_t size, const void* data) {
      vkCmdPushConstants(
         this->handle,
         m.pipeline.layout,
         flags,
         0,
         size,
         data
      );
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