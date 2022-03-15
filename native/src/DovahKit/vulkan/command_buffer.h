#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class material;
   class render_pass;
   class surface_renderer;

   class command_buffer : no_copy {
      protected:
         VkDevice      owning_device = VK_NULL_HANDLE;
         VkCommandPool owning_pool   = VK_NULL_HANDLE;
      public:
         command_buffer() {}
         command_buffer(VkDevice, VkCommandPool);
         command_buffer(surface_renderer&);
         ~command_buffer();

         static command_buffer create_transient(surface_renderer&);

         command_buffer(command_buffer&&) noexcept;
         command_buffer& operator=(command_buffer&&) noexcept;

         VkCommandBuffer handle = VK_NULL_HANDLE;

         static std::vector<command_buffer> create_in_bulk(VkDevice, VkCommandPool, size_t count);
         static std::vector<command_buffer> create_in_bulk(surface_renderer&, size_t count);

         VkResult reset(VkCommandBufferResetFlags);
         VkResult top_level_begin(VkCommandBufferUsageFlags);
         VkResult finish();
         
      protected:
         void _begin_render_pass(VkFramebuffer, const VkRect2D render_area, const render_pass&, const VkClearValue*, size_t clear_value_count, VkSubpassContents);
      public:
         template<size_t S> void begin_render_pass(const render_pass& rp, VkFramebuffer fb, const VkRect2D render_area, const std::array<VkClearValue, S>& clears, VkSubpassContents sc) {
            this->_begin_render_pass(fb, render_area, rp, clears.data(), clears.size(), sc);
         }
         inline void end_render_pass() {
            vkCmdEndRenderPass(this->handle);
         }
         
      protected:
         void _bind_material_and_descriptors(const material&, VkPipelineBindPoint, uint32_t bind_to, const VkDescriptorSet*, size_t ds_count, const uint32_t* dynamic_offsets, size_t do_count);
      public:
         template<size_t Sa, size_t Sb = 0> void bind_descriptor_sets(VkPipelineBindPoint bind_point, VkPipelineLayout layout, uint32_t bind_to, const std::array<VkDescriptorSet, Sa>& descriptor_sets, const std::array<uint32_t, Sb>& dynamic_offsets = {}) {
            vkCmdBindDescriptorSets(this->handle, bind_point, layout, bind_to, (uint32_t)descriptor_sets.size(), descriptor_sets.data(), (uint32_t)dynamic_offsets.size(), dynamic_offsets.data());
         }
         template<size_t Sa, size_t Sb = 0> void bind_material_and_descriptors(
            const material& mat,
            VkPipelineBindPoint bind_point,
            uint32_t bind_to,
            const std::array<VkDescriptorSet, Sa>& ds,
            const std::array<uint32_t, Sb>& dynamic_offsets = {}
         ) {
            this->_bind_material_and_descriptors(mat, bind_point, bind_to, ds.data(), ds.size(), dynamic_offsets.data(), dynamic_offsets.size());
         }

      protected:
         void _set_pipeline_push_constant(const material&, VkShaderStageFlags, size_t size, const void* data);
      public:
         template<typename PC> void set_pipeline_push_constant(const material& m, VkShaderStageFlags flags, const PC& data) {
            this->_set_pipeline_push_constant(m, flags, sizeof(PC), &data);
         }

      protected:
         void _setup();
   };
}