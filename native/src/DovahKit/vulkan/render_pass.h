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
            uint32_t view_mask = 0; // non-zero enables multiview
         };

         struct dependency {
            struct {
               uint32_t             subpass;
               VkAccessFlags        access_mask = 0;
               VkPipelineStageFlags stage_mask  = 0;
            } source;
            struct {
               uint32_t             subpass;
               VkAccessFlags        access_mask = 0;
               VkPipelineStageFlags stage_mask  = 0;
            } destination;
            //
            VkDependencyFlags flags = 0;
            int32_t view_offset = 0; // for multiview: given a VK_DEPENDENCY_VIEW_LOCAL_BIT dependency, each view X in the destination depends on view (X + view_offset) from the source, if the source has one.

            constexpr dependency() {}
            constexpr dependency(const VkSubpassDependency& data) {
               this->source = {
                  .subpass     = data.srcSubpass,
                  .access_mask = data.srcAccessMask,
                  .stage_mask  = data.srcStageMask,
               };
               this->destination = {
                  .subpass     = data.dstSubpass,
                  .access_mask = data.dstAccessMask,
                  .stage_mask  = data.dstStageMask,
               };
               this->flags = data.dependencyFlags;
            }

            constexpr VkSubpassDependency to_vulkan() const {
               return VkSubpassDependency{
                  .srcSubpass      = this->source.subpass,
                  .dstSubpass      = this->destination.subpass,
                  .srcStageMask    = this->source.stage_mask,
                  .dstStageMask    = this->destination.stage_mask,
                  .srcAccessMask   = this->source.access_mask,
                  .dstAccessMask   = this->destination.access_mask,
                  .dependencyFlags = this->flags,
               };
            }
         };

         // 
         // The start of a subpass  has an implicit  task: transitioning  the target image's 
         // current layout  to the one specified by the relevant attachment's  initialLayout 
         // field above. We of course need to ensure that the image in question (typically a 
         // swap chain image) is actually available (i.e. has been acquired) before any such 
         // transition is attempted.
         // 
         // If you don't specify a first external dependency  -- that is, a dependency whose 
         // source is  VK_SUBPASS_EXTERNAL -- then Vulkan  will inject a default  with these 
         // settings.
         // 
         static constexpr dependency make_default_leading_dependency(uint32_t first_using_subpass); // function exists mainly for documentation, really
         
         //
         // If you don't specify a final external dependency  -- that is, a dependency whose 
         // destination  is VK_SUBPASS_EXTERNAL  -- then  Vulkan will inject one  with these 
         // settings.
         //
         // Typically, if  an attachment's  finalLayout  (specified above)  differs from the 
         // layout  that the attachment has at the  end of your last subpass, you  will need 
         // to specify  your own final  external dependency;  the default one  won't be good 
         // enough. If you're able to rely on semaphores,  though, then the default can work 
         // even in that case.
         //
         static constexpr dependency make_default_trailing_dependency(uint32_t last_subpass); // function exists mainly for documentation, really

      public:
         render_pass(surface_renderer&);
         ~render_pass();

         render_pass(render_pass&&) noexcept;

         surface_renderer& owner;
         //
         std::vector<VkAttachmentDescription> attachments;
         struct {
            //
            // Multiview functionality allows a single subpass to write to multiple layers in the 
            // currently-bound framebuffer. Each subpass specifies a bitmask indicating which layers 
            // it writes to.
            //
            std::vector<uint32_t> correlation_masks; // list of bitmasks indicating which views, if any, have high spatial coherence (allows for optimization)
         } multiview_metadata;
         std::vector<subpass>    subpasses;
         std::vector<dependency> subpass_dependencies;
         //
         VkRenderPass handle = VK_NULL_HANDLE;

         void setup();
         void teardown();

         bool uses_multiview_functionality() const;
   };
}
