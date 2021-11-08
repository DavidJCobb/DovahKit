#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class context;

   class render_pass : no_copy {
      public:
         struct subpass {
            VkPipelineBindPoint bind_point;
            struct {
               std::vector<VkAttachmentReference> color;
               VkAttachmentReference depth_stencil;
            } attachments;
         };


      public:
         render_pass(context&);
         ~render_pass();

         render_pass(render_pass&&) noexcept;

         context& owner;
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
