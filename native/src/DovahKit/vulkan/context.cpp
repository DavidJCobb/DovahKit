#include "context.h"
#include "material.h"

namespace {
   static constexpr auto   desired_swap_chain_presentation_mode = VK_PRESENT_MODE_MAILBOX_KHR;
   static constexpr size_t frame_in_flight_count = 2;
}

namespace DovahKit::vulkan {
   #pragma region swap_chain
      void swap_chain::setup() {
         this->setup_basics();
         this->setup_views();
         this->setup_render_pass();
         this->setup_pipeline();
         this->setup_depth_buffer();
         this->setup_framebuffers();
         this->setup_uniform_buffers();
         this->setup_descriptor_pool();
         this->setup_descriptor_sets();
         this->setup_command_buffers();
         static_assert(false, "finish me");
      }
      void swap_chain::teardown() {
         assert(this->owner);
         auto logical = this->owner->devices.logical;
         //
         static_assert(false, "finish me");
         #pragma region pipeline
            vkDestroyPipeline      (logical, this->pipeline.handle, nullptr);
            vkDestroyPipelineLayout(logical, this->pipeline.layout, nullptr);
            this->pipeline.handle = VK_NULL_HANDLE;
            this->pipeline.layout = VK_NULL_HANDLE;
         #pragma endregion
         #pragma region render pass
            vkDestroyRenderPass(logical, this->render_pass, nullptr);
            this->render_pass = VK_NULL_HANDLE;
         #pragma endregion
         #pragma region views
            for (auto view : this->views) {
               vkDestroyImageView(logical, view, nullptr);
            }
            this->views.clear();
         #pragma endregion
         #pragma region basics
            vkDestroySwapchainKHR(logical, this->handle, nullptr);
            this->handle = VK_NULL_HANDLE;
         #pragma endregion
      }

      #pragma region setup steps
         void swap_chain::setup_basics() {
            assert(this->owner);
            assert(this->owner->surface);
            //
            auto* surface  = this->owner->surface;
            auto  logical  = this->owner->devices.logical;
            auto  physical = this->owner->devices.physical;
            auto  deets    = swap_chain_support_info(physical, *surface);
            //
            VkSurfaceFormatKHR surfaceFormat;
            VkPresentModeKHR   presentMode;
            VkExtent2D         extent;
            uint32_t           imageCount;
            //
            #pragma region choose format
               assert(!deets.formats.empty());
               surfaceFormat = deets.formats[0]; // fallback
               for (const auto& current_format : deets.formats) {
                  if (current_format.format == VK_FORMAT_B8G8R8A8_SRGB && current_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                     surfaceFormat = current_format;
                     break;
                  }
               }
               this->format = surfaceFormat.format;
            #pragma endregion
            #pragma region choose presentation mode
               presentMode = VK_PRESENT_MODE_FIFO_KHR; // fallback
               for (const auto& current_mode : deets.presentation_modes) {
                  if (current_mode == desired_swap_chain_presentation_mode) {
                     presentMode = current_mode;
                     break;
                  }
               }
            #pragma endregion
            #pragma region choose extent
               if (deets.capabilities.currentExtent.width != UINT32_MAX) {
                  this->extent = deets.capabilities.currentExtent;
               } else {
                  auto& min_e = deets.capabilities.minImageExtent;
                  auto& max_e = deets.capabilities.maxImageExtent;
                  //
                  this->extent = surface->extent();
                  this->extent.width  = std::clamp(this->extent.width,  min_e.width,  max_e.width);
                  this->extent.height = std::clamp(this->extent.height, min_e.height, max_e.height);
               }
            #pragma endregion
            #pragma region choose image count
               imageCount = deets.capabilities.minImageCount + 1;
               if (deets.capabilities.maxImageCount > 0 && imageCount > deets.capabilities.maxImageCount) {
                  imageCount = deets.capabilities.maxImageCount;
               }
            #pragma endregion
            //
            auto create_info = VkSwapchainCreateInfoKHR{
               .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
               .surface          = surface->handle,
               .minImageCount    = imageCount,
               .imageFormat      = surfaceFormat.format,
               .imageColorSpace  = surfaceFormat.colorSpace,
               .imageExtent      = extent,
               .imageArrayLayers = 1,
               .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            };
            //
            auto indices = QueueFamilies(physical);
            auto list    = indices.families.list;
            if (indices.families.graphics != indices.families.presentation) {
               //
               // TODO: Apparently "exclusive" is faster for this case, but requires more complicated setup, 
               //       which the tutorial I'm following feels should be saved for later.
               // 
               // See: https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain#page_Creating-the-swap-chain
               //
               create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
               create_info.queueFamilyIndexCount = list.size();
               create_info.pQueueFamilyIndices   = list.data();
            } else {
               create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
               create_info.queueFamilyIndexCount = 0;       // clearing these two values is optional, but feels cleaner to me
               create_info.pQueueFamilyIndices   = nullptr; //
            }
            create_info.preTransform   = deets.capabilities.currentTransform; // don't rotate or otherwise transform the image while rendering
            create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;   // disable alpha
            create_info.presentMode    = presentMode;
            create_info.clipped        = VK_TRUE;        // disable rendering of pixels covered (e.g. by other windows); good optimization, but prevents querying the colors of those pixels (e.g. for saving snapshots)
            create_info.oldSwapchain   = VK_NULL_HANDLE; // must be specified when rebuilding a swap chain; keep null for making a new swap chain
            //
            if (vkCreateSwapchainKHR(logical, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
               throw std::runtime_error("[DovahKit::vulkan::swap_chain::setup_basics] Failed to create the swap chain handle.");
            }
            //
            // Get swap chain images, to render to later:
            //
            vkGetSwapchainImagesKHR(logical, this->handle, &imageCount, nullptr);
            this->images.resize(imageCount);
            vkGetSwapchainImagesKHR(logical, this->handle, &imageCount, this->images.data());
         }
         void swap_chain::setup_views() {
            assert(this->owner);
            auto& device = this->owner->devices;
            auto  count  = this->image_count();
            //
            this->views.resize(count);
            for (size_t i = 0; i < count; i++) {
               this->views[i] = device.create_image_view(this->images[i], this->format, VK_IMAGE_ASPECT_COLOR_BIT);
            }
         }
         void swap_chain::setup_render_pass() {
            assert(this->owner);
            auto& device = this->owner->devices;
            //
            std::array attachment_descs = {
               VkAttachmentDescription{ // color
                  .format         = this->format,
                  .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
                  .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                  .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                  .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                  .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                  .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                  .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
               },
               VkAttachmentDescription{ // depth
                  .format         = device.find_depth_format(),
                  .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
                  .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                  .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after drawing, so let the driver decide how best to discard it
                  .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                  .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                  .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                  .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               },
            };
            std::array attachment_refs = {
               VkAttachmentReference{ // color
                  .attachment = 0,
                  .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               },
               VkAttachmentReference{ // depth
                  .attachment = 1,
                  .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               },
            };
            //
            auto subpass = VkSubpassDescription{
               .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .colorAttachmentCount    = 1,
               .pColorAttachments       = &attachment_refs[0],
               .pDepthStencilAttachment = &attachment_refs[1], // subpasses can only use a single depth-and-stencil attachment
            };
            auto dependency = VkSubpassDependency{
               .srcSubpass    = VK_SUBPASS_EXTERNAL,
               .dstSubpass    = 0,
               .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask = 0,
               .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            };
            //
            auto render_pass_info = VkRenderPassCreateInfo{
               .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
               .attachmentCount = attachment_descs.size(),
               .pAttachments    = attachment_descs.data(),
               .subpassCount    = 1,
               .pSubpasses      = &subpass,
               .dependencyCount = 1,
               .pDependencies   = &dependency,
            };
            if (vkCreateRenderPass(this->owner->devices.logical, &render_pass_info, nullptr, &this->render_pass) != VK_SUCCESS) {
               throw std::runtime_error("[DovahKit::vulkan::swap_chain::setup_render_pass] Failed to create render pass.");
            }
         }
         void swap_chain::setup_pipeline() {
            assert(this->owner);
            auto& device = this->owner->devices;
            //
            auto shaderStages = std::array{ frag_info, vert_info };
            //
            auto vert_binding    = vertex::getBindingDescription();
            auto vert_attributes = vertex::getAttributeDescriptions();
            auto visc = VkPipelineVertexInputStateCreateInfo{ // describes the format of vertex info to be passed to the vertex shader
               .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
               .vertexBindingDescriptionCount   = 1,
               .pVertexBindingDescriptions      = &vert_binding,
               .vertexAttributeDescriptionCount = (uint32_t)vert_attributes.size(),
               .pVertexAttributeDescriptions    = vert_attributes.data(),
            };
            auto iasc = VkPipelineInputAssemblyStateCreateInfo{ // describes how to generate triangles from the vertices we're passing in
               .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
               .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
               .primitiveRestartEnable = VK_FALSE,
            };
            //
            auto viewport = VkViewport{ // describe what part of the framebuffer we should draw to
               .x        = 0.0,
               .y        = 0.0,
               .width    = (float)this->extent.width,
               .height   = (float)this->extent.height,
               .minDepth = 0.0, // must be >= 0
               .maxDepth = 1.0, // must be <= 1
            };
            auto scissor = VkRect2D{ // describe what part of the framebuffer we should retain (like a write-mask)
               .offset = {0, 0},
               .extent = this->extent,
            };
            auto viewport_create = VkPipelineViewportStateCreateInfo{
               .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
               .viewportCount = 1,
               .pViewports    = &viewport,
               .scissorCount  = 1,
               .pScissors     = &scissor,
            };
            //
            auto rasterizer_create = VkPipelineRasterizationStateCreateInfo{
               .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
               .depthClampEnable        = VK_FALSE,
               .rasterizerDiscardEnable = VK_FALSE, // setting this to true basically disables the rasterizer entirely
               .polygonMode             = VK_POLYGON_MODE_FILL,
               .cullMode                = VK_CULL_MODE_BACK_BIT,   // cull backfaces, frontfaces (why? lol), or no faces
               .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE, // specify which vertex order (clockwise or counterclockwise) signifies a face pointing toward us
               .depthBiasEnable         = VK_FALSE,
               .depthBiasConstantFactor = 0.0f,
               .depthBiasClamp          = 0.0f,
               .depthBiasSlopeFactor    = 0.0f,
               .lineWidth               = 1.0,
            };
            auto multisample_info = VkPipelineMultisampleStateCreateInfo{ // MSAA (multisampling anti-alias); just disable it
               .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
               .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
               .sampleShadingEnable   = VK_FALSE,
               .minSampleShading      = 1.0F,
               .pSampleMask           = nullptr,
               .alphaToCoverageEnable = VK_FALSE,
               .alphaToOneEnable      = VK_FALSE,
            };
            auto color_blend_attach_info = VkPipelineColorBlendAttachmentState{
               //
               // When drawing onto the framebuffer, how should we paint overtop the previous image?
               //
               .blendEnable         = VK_FALSE,
               .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // These are basically like "alpha" values for traditional blending, but they're only used 
               .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // to influence the RGB color components.
               .colorBlendOp        = VK_BLEND_OP_ADD,
               .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // These are basically like "alpha" values for traditional blending, but they're only used 
               .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // to influence the A color component.
               .alphaBlendOp        = VK_BLEND_OP_ADD,
               .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };
            auto color_blend_create_info = VkPipelineColorBlendStateCreateInfo{
               .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
               .logicOpEnable   = VK_FALSE,         // Enables bitwise-operation blending. Mutually exclusive with "attachment state" 
               .logicOp         = VK_LOGIC_OP_COPY, // blending and will disable that.
               .attachmentCount = 1,
               .pAttachments    = &color_blend_attach_info,
               .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
            };
            auto depth_stencil_attach_info = VkPipelineDepthStencilStateCreateInfo{
               .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
               .depthTestEnable       = VK_TRUE,
               .depthWriteEnable      = VK_TRUE,
               .depthCompareOp        = VK_COMPARE_OP_LESS, // lower depth value = closer
               .depthBoundsTestEnable = VK_FALSE, // toggle whether values outside of a depth range are culled
               .stencilTestEnable     = VK_FALSE,
               .front                 = {}, // stencil info
               .back                  = {}, // stencil info
               .minDepthBounds        = 0.0, // depth culling range
               .maxDepthBounds        = 1.0, // depth culling range
            };
            //
            // Now let's create the pipeline layout.
            //
            auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
               .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
               .setLayoutCount         = 1,
               .pSetLayouts            = &this->descriptor_set_layout,
               .pushConstantRangeCount = 0,
               .pPushConstantRanges    = nullptr,
            };
            if (vkCreatePipelineLayout(device.logical, &pipeline_layout_info, nullptr, &this->pipeline.layout) != VK_SUCCESS) {
               throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create pipeline layout.");
            }
            //
            // Next, the pipeline itself.
            //
            auto pipeline_info = VkGraphicsPipelineCreateInfo{
               .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
               .stageCount = (uint32_t)shaderStages.size(),
               .pStages    = shaderStages.data(), 
               //
               .pVertexInputState   = &visc,
               .pInputAssemblyState = &iasc,
               .pViewportState      = &viewport_create,
               .pRasterizationState = &rasterizer_create,
               .pMultisampleState   = &multisample_info,
               .pDepthStencilState  = &depth_stencil_attach_info,
               .pColorBlendState    = &color_blend_create_info,
               .pDynamicState       = nullptr,
               //
               .layout = this->pipeline.layout,
               //
               .renderPass = this->render_pass,
               .subpass    = 0,
               //
               .basePipelineHandle = VK_NULL_HANDLE,
               .basePipelineIndex = -1,
            };
            if (vkCreateGraphicsPipelines(device.logical, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->pipeline.handle) != VK_SUCCESS) {
               throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create graphics pipeline.");
            }
         }
      #pragma endregion
   #pragma endregion
}