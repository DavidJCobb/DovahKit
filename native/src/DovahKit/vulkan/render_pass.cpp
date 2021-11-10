#include "render_pass.h"
#include "surface_renderer.h"

namespace vulkanDK {
   render_pass::render_pass(surface_renderer& c) : owner(c) {
   }
   render_pass::~render_pass() {
      this->teardown();
   }

   render_pass::render_pass(render_pass&& o) noexcept : owner(o.owner) {
      std::swap(this->attachments, o.attachments);
      std::swap(this->subpasses,   o.subpasses);
      std::swap(this->handle,      o.handle);
   }

   void render_pass::setup() {
      assert(this->handle == VK_NULL_HANDLE);
      //
      uint32_t subpass_desc_count = this->subpasses.descriptions.size();
      std::vector<VkSubpassDescription> subpass_descriptions(subpass_desc_count);
      for (uint32_t i = 0; i < subpass_desc_count; ++i) {
         auto& src = this->subpasses.descriptions[i];
         auto& dst = subpass_descriptions[i];
         //
         subpass_descriptions[i] = VkSubpassDescription{
            .pipelineBindPoint       = src.bind_point,
            .colorAttachmentCount    = (uint32_t)src.attachments.color.size(),
            .pColorAttachments       = src.attachments.color.data(),
            .pDepthStencilAttachment = &src.attachments.depth_stencil, // subpasses can only use a single depth-and-stencil attachment
         };
      }
      auto render_pass_info = VkRenderPassCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
         .attachmentCount = (uint32_t)this->attachments.size(),
         .pAttachments    = this->attachments.data(),
         .subpassCount    = subpass_desc_count,
         .pSubpasses      = subpass_descriptions.data(),
         .dependencyCount = (uint32_t)this->subpasses.dependencies.size(),
         .pDependencies   = this->subpasses.dependencies.data(),
      };
      if (vkCreateRenderPass(this->owner.logical_device, &render_pass_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::render_pass] Failed to create render pass.");
      }
   }
   void render_pass::teardown() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      vkDestroyRenderPass(this->owner.logical_device, this->handle, nullptr);
   }
}