#include "render_pass.h"
#include "exceptions.h"
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

   /*static*/ constexpr render_pass::dependency render_pass::make_default_leading_dependency(uint32_t first_using_subpass) {
      return VkSubpassDependency{
         .srcSubpass      = VK_SUBPASS_EXTERNAL,
         .dstSubpass      = first_using_subpass, /* Vulkan uses the first subpass that the attachment is used in */
         .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
         .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
         .srcAccessMask   = VK_ACCESS_NONE_KHR,
         .dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
         .dependencyFlags = 0,
      };
   }
   /*static*/ constexpr render_pass::dependency render_pass::make_default_trailing_dependency(uint32_t last_subpass) {
      return VkSubpassDependency{
         .srcSubpass      = last_subpass, // should be the last subpass in the list
         .dstSubpass      = VK_SUBPASS_EXTERNAL,
         .srcStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
         .dstStageMask    = VK_PIPELINE_STAGE_NONE_KHR, // wait until full command buffer is done
         .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .dstAccessMask   = VK_ACCESS_NONE_KHR,
         .dependencyFlags = 0,
      };
   }

   void render_pass::setup() {
      assert(this->handle == VK_NULL_HANDLE);
      //
      const uint32_t subpass_desc_count   = this->subpasses.size();
      const uint32_t subpass_depend_count = this->subpass_dependencies.size();
      const bool     enable_multiview     = this->uses_multiview_functionality();
      //
      std::vector<uint32_t> multiview_view_masks(enable_multiview ? subpass_desc_count : 0);
      std::vector<VkSubpassDescription> subpass_descriptions(subpass_desc_count);
      for (uint32_t i = 0; i < subpass_desc_count; ++i) {
         const auto& src = this->subpasses[i];
         auto&       dst = subpass_descriptions[i];
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
         //
         if (enable_multiview)
            multiview_view_masks[i] = src.view_mask;
      }
      //
      std::vector<int32_t>  multiview_view_offsets;
      std::vector<VkSubpassDependency> vk_dependencies(subpass_depend_count);
      for (size_t i = 0; i < subpass_depend_count; ++i) {
         auto& item = this->subpass_dependencies[i];
         //
         vk_dependencies[i] = item.to_vulkan();
         if (enable_multiview) {
            if (item.view_offset || !multiview_view_offsets.empty()) {
               if (multiview_view_offsets.empty()) {
                  multiview_view_offsets.resize(subpass_depend_count);
               }
               multiview_view_offsets[i] = item.view_offset;
            }
         }
      }
      //
      auto multiview_info = VkRenderPassMultiviewCreateInfo{
         .sType                = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
         .pNext                = nullptr,
         .subpassCount         = subpass_desc_count,
         .pViewMasks           = multiview_view_masks.data(), // one per subpass
         .dependencyCount      = (uint32_t)multiview_view_offsets.size(), // zero, or number of dependencies in the render pass
         .pViewOffsets         = multiview_view_offsets.data(),
         .correlationMaskCount = (uint32_t)this->multiview_metadata.correlation_masks.size(),
         .pCorrelationMasks    = this->multiview_metadata.correlation_masks.data(),
      };
      auto render_pass_info = VkRenderPassCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
         .pNext           = enable_multiview ? &multiview_info : nullptr,
         .flags           = 0,
         .attachmentCount = (uint32_t)this->attachments.size(),
         .pAttachments    = this->attachments.data(),
         .subpassCount    = subpass_desc_count,
         .pSubpasses      = subpass_descriptions.data(),
         .dependencyCount = subpass_depend_count,
         .pDependencies   = vk_dependencies.data(),
      };
      if (auto result = vkCreateRenderPass(this->owner.logical_device, &render_pass_info, nullptr, &this->handle); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::render_pass] Failed to create render pass.");
      }
   }
   void render_pass::teardown() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      vkDestroyRenderPass(this->owner.logical_device, this->handle, nullptr);
   }

   bool render_pass::uses_multiview_functionality() const {
      for (const auto& subpass : this->subpasses)
         if (subpass.view_mask)
            return true;
      return false;
   }
}