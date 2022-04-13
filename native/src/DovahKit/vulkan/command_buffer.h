#pragma once
#include <array>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "abstract_renderer.h"

namespace vulkanDK {
   class compute_shader;
   class graphics_shader;
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
         command_buffer(surface_renderer&, abstract_renderer::queue&);
         ~command_buffer();

         static command_buffer create_transient(surface_renderer&);
         static command_buffer create_transient(surface_renderer&, abstract_renderer::queue&);

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
         void next_render_subpass() {
            vkCmdNextSubpass(this->handle, VK_SUBPASS_CONTENTS_INLINE);
         }
         inline void end_render_pass() {
            vkCmdEndRenderPass(this->handle);
         }

      protected:
         void _bind_compute_shader_and_descriptors(const compute_shader&, uint32_t bind_to, const VkDescriptorSet*, size_t ds_count, const uint32_t* dynamic_offsets, size_t do_count);
      public:
         template<size_t Sa, size_t Sb = 0> void bind_compute_shader_and_descriptors(const compute_shader& cs, uint32_t bind_to, const std::array<VkDescriptorSet, Sa>& descriptor_sets, const std::array<uint32_t, Sb>& dynamic_offsets = {}) {
            this->_bind_compute_shader_and_descriptors(cs, bind_to, descriptor_sets.data(), descriptor_sets.size(), dynamic_offsets.data(), dynamic_offsets.size());
         }
         
      protected:
         void _bind_graphics_shader_and_descriptors(const graphics_shader&, VkPipelineBindPoint, uint32_t bind_to, const VkDescriptorSet*, size_t ds_count, const uint32_t* dynamic_offsets, size_t do_count);
      public:
         template<size_t Sa, size_t Sb = 0> void bind_descriptor_sets(VkPipelineBindPoint bind_point, VkPipelineLayout layout, uint32_t bind_to, const std::array<VkDescriptorSet, Sa>& descriptor_sets, const std::array<uint32_t, Sb>& dynamic_offsets = {}) {
            vkCmdBindDescriptorSets(this->handle, bind_point, layout, bind_to, (uint32_t)descriptor_sets.size(), descriptor_sets.data(), (uint32_t)dynamic_offsets.size(), dynamic_offsets.data());
         }
         template<size_t Sa, size_t Sb = 0> void bind_graphics_shader_and_descriptors(
            const graphics_shader& gs,
            VkPipelineBindPoint bind_point,
            uint32_t bind_to,
            const std::array<VkDescriptorSet, Sa>& ds,
            const std::array<uint32_t, Sb>& dynamic_offsets = {}
         ) {
            this->_bind_graphics_shader_and_descriptors(gs, bind_point, bind_to, ds.data(), ds.size(), dynamic_offsets.data(), dynamic_offsets.size());
         }

      protected:
         void _set_pipeline_push_constant(VkPipelineLayout, VkShaderStageFlags, size_t size, const void* data);
      public:
         template<typename PC> void set_pipeline_push_constant(VkPipelineLayout layout, VkShaderStageFlags flags, const PC& data) {
            this->_set_pipeline_push_constant(layout, flags, sizeof(PC), &data);
         }

      protected:
         void _setup();
   };
}