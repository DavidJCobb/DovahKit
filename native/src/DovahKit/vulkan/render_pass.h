#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class surface_renderer;

   class render_pass : no_copy {
      public:
         struct subpass {
            VkPipelineBindPoint bind_point;
            struct {
               std::vector<VkAttachmentReference> color;
               VkAttachmentReference depth_stencil;
               std::vector<VkAttachmentReference> input;
            } attachments;
         };


      public:
         render_pass(surface_renderer&);
         ~render_pass();

         render_pass(render_pass&&) noexcept;

         surface_renderer& owner;
         //
         std::vector<VkAttachmentDescription> attachments;
         struct {
            std::vector<subpass> descriptions;
            std::vector<VkSubpassDependency> dependencies;
         } subpasses;
         //
         VkRenderPass handle = VK_NULL_HANDLE;

         void setup();
         void teardown();
   };
}
