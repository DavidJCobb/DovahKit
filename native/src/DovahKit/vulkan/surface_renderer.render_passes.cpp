#include "surface_renderer.h"
#include "./config/order_independent_transparency.h"
#include "./render_pass.h"

namespace vulkanDK {
   void surface_renderer::_define_render_passes() {
      {
         auto* rp = this->render_passes_by_name.main_shadow = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE, // we won't use this data after subpass 0, where it's generated, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
         };
         rp->subpasses = {
            {  // subpass
               .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 0,
                     .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // should be the destination of the last dependency?
               .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // wait until the fragment shader
               .srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }
         };
      }
      {
         auto* rp = this->render_passes_by_name.main_shadow_placed = new render_pass(*this);
         {
            auto desc = VkAttachmentDescription{
               .format         = VK_FORMAT_R32_SFLOAT,
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE, // we won't use this data after subpass 0, where it's generated, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            //
            auto& list = rp->attachments;
            list.resize(shadow_caster_count);
            for(auto& item : list)
               item = desc;
         }
         {
            auto& list = rp->subpasses;
            list.resize(shadow_caster_count);
            for (size_t i = 0; i < shadow_caster_count; ++i) {
               list[i] = {  // subpass
                  .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                  .attachments = {
                     .color = {
                        VkAttachmentReference{
                           .attachment = (uint32_t)i,
                           .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        },
                     },
                  },
                  .view_mask = 0b00111111,
               };
            }
         }
         rp->subpass_dependencies = {
            VkSubpassDependency{
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = 0,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            },
         };
      }
      {
         auto* rp = this->render_passes_by_name.main = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               //.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // for OIT
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
            },
         };
         rp->subpasses = {
            {  // subpass
               .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = { // there can be multiple color attachments
                     VkAttachmentReference{
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 1,
                     .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            //
            // A subpass dependency specifies that  certain tasks in the "source" must complete 
            // before other  tasks in the  "destination" are allowed  to proceed.  The "source" 
            // subpass must always precede (have a lower index than) the "destination" subpass. 
            // The special subpass index VK_SUBPASS_EXTERNAL  refers to tasks occurring outside 
            // of the render pass --  the start (source) or end (destination) of a render pass, 
            // as it were.
            // 
            // For a subpass  dependency, a "task" is a  read or write  operation (access mask) 
            // and the stages in which that operation occurs (stage mask). Because you can list 
            // multiple stages, many accesses are defined on a per-stage basis (e.g. a specific 
            // flag for "reading the color attachment,"  rather than a single flag for "read").
            // 
            // The start of a subpass  has an implicit  task: transitioning  the target image's 
            // current layout  to the one specified by the relevant attachment's  initialLayout 
            // field above. We of course need to ensure that the image in question (typically a 
            // swap chain image) is actually available (i.e. has been acquired) before any such 
            // transition is attempted.
            // 
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = 0,
               .dstAccessMask   = (VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT) | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // should be the destination of the last dependency?
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }
         };
      }
      if (this->can_do_alpha()) {
         auto* rp = this->render_passes_by_name.main_oit = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // OIT accumulator
               .format         = config::format_for_oit_accumulator, // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // the initial layout is the EXPECTED state, not something that is done for you. YOU have to get it there before starting.
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // the final layout IS done for you.
            },
            VkAttachmentDescription{ // OIT reveal
               .format         = config::format_for_oit_reveal,
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
            },
         };
         rp->subpasses = {
            {  // subpass: color pass
               .bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = {
                     VkAttachmentReference{ // OIT accumulator
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     },
                     VkAttachmentReference{ // OIT reveal
                        .attachment = 1,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 3,
                     .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
            {  // subpass: composite pass
               .bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = {
                     VkAttachmentReference{ // color
                        .attachment = 2,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     },
                  },
                  .input = {
                     VkAttachmentReference{ // OIT accumulator
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     },
                     VkAttachmentReference{ // OIT reveal
                        .attachment = 1,
                        .layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     }
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               //
               // Wait for the initial layout transition to complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = 0, // 0 == all operations? documentation/spec are unclear
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               //
               // Wait for the layout transition from subpass 0 to 1 to complete? Or wait 
               // for writes from the previous subpass to complete before allowing reads?
               //
               .srcSubpass      = 0,
               .dstSubpass      = 1,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
            },
            /*//
            VkSubpassDependency{ // dependency to transition the images back to optimal
               .srcSubpass      = 1,
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            //*/
         };
      }
      {
         auto* rp = this->render_passes_by_name.bounds = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after this render pass, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
         };
         rp->subpasses = {
            {  // subpass
               .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = { // there can be multiple color attachments
                     VkAttachmentReference{
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 1,
                     .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            }
         };
      }
      {
         auto* rp = this->render_passes_by_name.gizmo = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR, // reset depth buffer
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after this render pass, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
         };
         rp->subpasses = {
            {  // subpass
               .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = { // there can be multiple color attachments
                     VkAttachmentReference{
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 1,
                     .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            }
         };
      }
      {
         auto* rp = this->render_passes_by_name.ui = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR, // reset depth buffer
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after subpass 0, where it's generated, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
            },
         };
         rp->subpasses = {
            {  // subpass
               .bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = { // there can be multiple color attachments
                     VkAttachmentReference{
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 1,
                     .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = 0, // 0 == all operations? documentation/spec are unclear
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // should be the destination of the last dependency?
               //.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // wait until end of pipeline
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }
         };
      }
      //
      // Register with abstract renderer:
      //
      auto& dst = this->render_passes;
      auto& src = this->render_passes_by_name._list;
      this->render_passes.resize(src.size());
      for (size_t i = 0; i < src.size(); ++i)
         dst[i] = src[i];
   }
   void surface_renderer::_setup_render_passes() {
      this->render_passes_by_name.main->attachments[0].format = this->swap_chain.format;
      if (auto* rp = this->render_passes_by_name.main_oit) {
         rp->attachments[2].format = this->swap_chain.format;
      }
      this->render_passes_by_name.bounds->attachments[0].format = this->swap_chain.format;
      this->render_passes_by_name.gizmo->attachments[0].format = this->swap_chain.format;
      this->render_passes_by_name.ui->attachments[0].format = this->swap_chain.format;
      //
      // (Re)create the render passes within the GPU:
      //
      for(auto* rp : this->render_passes)
         if (rp)
            rp->setup();
      //
      this->set_debug_object_name(this->render_passes_by_name.main_shadow->handle, "Render Pass: Sun Shadows");
      this->set_debug_object_name(this->render_passes_by_name.main_shadow_placed->handle, "Render Pass: Caster Shadows");
      this->set_debug_object_name(this->render_passes_by_name.main->handle, "Render Pass: Main");
      if (auto* rp = this->render_passes_by_name.main_oit) {
         this->set_debug_object_name(rp->handle, "Render Pass: Main OIT");
      }
      this->set_debug_object_name(this->render_passes_by_name.bounds->handle, "Render Pass: Bounds");
      this->set_debug_object_name(this->render_passes_by_name.gizmo->handle, "Render Pass: Gizmo");
      this->set_debug_object_name(this->render_passes_by_name.ui->handle,   "Render Pass: UI");
   }
}