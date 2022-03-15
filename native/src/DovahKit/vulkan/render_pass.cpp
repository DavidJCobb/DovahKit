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
         const auto* depth = &src.attachments.depth_stencil; // subpasses can only use a single depth-and-stencil attachment... but may not use one
         if (src.attachments.depth_stencil.layout == VK_IMAGE_LAYOUT_UNDEFINED)
            depth = nullptr;
         //
         subpass_descriptions[i] = VkSubpassDescription{
            .flags                   = 0,
            .pipelineBindPoint       = src.bind_point,
            .inputAttachmentCount    = (uint32_t)src.attachments.input.size(),
            .pInputAttachments       = src.attachments.input.data(),
            .colorAttachmentCount    = (uint32_t)src.attachments.color.size(),
            .pColorAttachments       = src.attachments.color.data(),
            .pResolveAttachments     = nullptr,
            .pDepthStencilAttachment = depth,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments    = nullptr,
         };
      }
      auto render_pass_info = VkRenderPassCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
         .pNext           = nullptr,
         .flags           = 0,
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