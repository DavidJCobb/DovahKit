#include "material.h"
#include <stdexcept>
#include "surface_renderer.h"

namespace vulkanDK {
   #pragma region material_definition
   material_definition::material_definition() {
      this->inputs.triangles = VkPipelineInputAssemblyStateCreateInfo{
         .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .pNext                  = nullptr,
         .flags                  = 0,
         .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
         .primitiveRestartEnable = VK_FALSE,
      };
      this->multisampling = VkPipelineMultisampleStateCreateInfo{ // MSAA (multisampling anti-alias); disable it by default (one sample only)
         .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
         .sampleShadingEnable   = VK_FALSE,
         .minSampleShading      = 1.0,
         .pSampleMask           = nullptr,
         .alphaToCoverageEnable = VK_FALSE,
         .alphaToOneEnable      = VK_FALSE,
      };
      this->rasterization = VkPipelineRasterizationStateCreateInfo{
         .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
         .depthClampEnable        = VK_FALSE,
         .rasterizerDiscardEnable = VK_FALSE, // setting this to true basically disables the rasterizer entirely
         .polygonMode             = VK_POLYGON_MODE_FILL,  // fill polygons, or render wireframes or point clouds?
         .cullMode                = VK_CULL_MODE_BACK_BIT, // cull backfaces, frontfaces, or no faces
         .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE, // specify which vertex order (clockwise or counterclockwise) signifies a face pointing toward us
         .depthBiasEnable         = VK_FALSE,
         .depthBiasConstantFactor = 0.0,
         .depthBiasClamp          = 0.0,
         .depthBiasSlopeFactor    = 0.0,
         .lineWidth               = 1.0, // line width, e.g. for wireframes
      };
   }

   void material_definition::add_stage(const stage_info& s) {
      this->stages.push_back(s);
   }

   std::vector<VkPipelineShaderStageCreateInfo> material_definition::stage_create_info() const {
      std::vector<VkPipelineShaderStageCreateInfo> out;
      //
      auto  size = this->stages.size();
      auto& list = this->stages;
      out.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& stage = list[i];
         out[i] = VkPipelineShaderStageCreateInfo{
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = stage.stage,
            .module = stage.module->handle,
            .pName  = stage.entry_point_name,
            .pSpecializationInfo = stage.specialization_info,
         };
      }
      return out;
   }
   //
   VkPipelineDepthStencilStateCreateInfo material_definition::depth_stencil_info() const {
      return VkPipelineDepthStencilStateCreateInfo{
         .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
         .depthTestEnable       = this->depth.testing,
         .depthWriteEnable      = this->depth.writing,
         .depthCompareOp        = this->depth.comparison,
         .depthBoundsTestEnable = this->depth.culling.enabled,
         .stencilTestEnable     = this->stencil.testing,
         .front                 = this->stencil.front,
         .back                  = this->stencil.back,
         .minDepthBounds        = this->depth.culling.min,
         .maxDepthBounds        = this->depth.culling.max,
      };
   }
   //
   std::vector<VkPipelineColorBlendAttachmentState> material_definition::color_blend_attachment_info() const {
      std::vector<VkPipelineColorBlendAttachmentState> out;
      auto& list = this->color_blending.blends;
      auto  size = list.size();
      out.resize(size);
      for (size_t i = 0; i < size; ++i) {
         const auto& item = list[i];
         out[i] = VkPipelineColorBlendAttachmentState{
            .blendEnable         = item.enabled ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = item.source.color,
            .dstColorBlendFactor = item.destination.color,
            .colorBlendOp        = item.operations.color,
            .srcAlphaBlendFactor = item.source.alpha,
            .dstAlphaBlendFactor = item.destination.alpha,
            .alphaBlendOp        = item.operations.alpha,
            .colorWriteMask      = item.write_mask,
         };
      }
      return out;
   }
   VkPipelineColorBlendStateCreateInfo material_definition::color_blend_info(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) const {
      return VkPipelineColorBlendStateCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
         .logicOpEnable   = VK_FALSE,         // Enables bitwise-operation blending. Mutually exclusive with "attachment state" 
         .logicOp         = VK_LOGIC_OP_COPY, // blending and will disable that.
         .attachmentCount = (uint32_t)attachments.size(),
         .pAttachments    = attachments.data(),
         .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
      };
   }

   VkPipelineVertexInputStateCreateInfo material_definition::vertex_info() const {
      auto& src = this->inputs.vertex;
      return VkPipelineVertexInputStateCreateInfo{
         .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
         .pNext                           = nullptr,
         .flags                           = src.flags,
         .vertexBindingDescriptionCount   = (uint32_t)src.bindings.size(),
         .pVertexBindingDescriptions      = src.bindings.data(),
         .vertexAttributeDescriptionCount = (uint32_t)src.attributes.size(),
         .pVertexAttributeDescriptions    = src.attributes.data(),
      };
   }
   #pragma endregion

   #pragma region material
   material::~material() {
      if (!this->owner)
         return;
      this->teardown_handle();
      if (this->pipeline.layout != VK_NULL_HANDLE) {
         vkDestroyPipelineLayout(this->owner->logical_device, this->pipeline.layout, nullptr);
         this->pipeline.layout = VK_NULL_HANDLE;
      }
   }

   material::material(material&& o) noexcept {
      std::swap(this->owner, o.owner);
      std::swap(this->pipeline.handle, o.pipeline.handle);
      std::swap(this->pipeline.layout, o.pipeline.layout);
   }
   material& material::operator=(material&& o) noexcept {
      std::swap(this->owner, o.owner);
      std::swap(this->pipeline.handle, o.pipeline.handle);
      std::swap(this->pipeline.layout, o.pipeline.layout);
      return *this;
   }

   void material::setup_layout(const std::vector<VkDescriptorSetLayout>& dsl, const std::vector<VkPushConstantRange>& pcr) {
      if (!this->owner) {
         throw std::logic_error("[vulkanDK::material::setup_layout] Owner required.");
      }
      auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
         .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount         = (uint32_t)dsl.size(),
         .pSetLayouts            = dsl.data(),
         .pushConstantRangeCount = (uint32_t)pcr.size(),
         .pPushConstantRanges    = pcr.data(),
      };
      if (vkCreatePipelineLayout(this->owner->logical_device, &pipeline_layout_info, nullptr, &this->pipeline.layout) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::material::setup_layout] Failed to create pipeline layout.");
      }
   }
   void material::setup_handle(const material_definition& def, VkViewport viewport, VkRect2D scissor, VkRenderPass render_pass_handle, uint32_t subpass) {
      if (!this->owner) {
         throw std::logic_error("[vulkanDK::material::setup_handle] Owner required.");
      }
      if (this->pipeline.layout == VK_NULL_HANDLE) {
         throw std::logic_error("[vulkanDK::material::setup_handle] You must set up the pipeline layout first.");
      }
      auto stages = def.stage_create_info();
      auto depth  = def.depth_stencil_info();
      auto vertex = def.vertex_info();
      //
      auto blends = def.color_blend_attachment_info();
      auto color  = def.color_blend_info(blends);

      auto viewport_create = VkPipelineViewportStateCreateInfo{
         .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1,
         .pViewports    = &viewport,
         .scissorCount  = 1,
         .pScissors     = &scissor,
      };

      //
      // TODO: If we take a render_pass& instead of a render pass handle, then we can verify that 
      // (color) has the right blend count for the specified subpass (it should match the value 
      // render_pass.subpasses[n].attachments.color.size()).
      // 
      // We could also validate that the subpass number itself is valid.
      //

      auto pipeline_info = VkGraphicsPipelineCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
         .stageCount = (uint32_t)stages.size(),
         .pStages    = stages.data(), 
         //
         .pVertexInputState   = &vertex,
         .pInputAssemblyState = &def.inputs.triangles,
         .pViewportState      = &viewport_create,
         .pRasterizationState = &def.rasterization,
         .pMultisampleState   = &def.multisampling,
         .pDepthStencilState  = &depth,
         .pColorBlendState    = &color,
         .pDynamicState       = nullptr,
         //
         .layout = this->pipeline.layout,
         //
         .renderPass = render_pass_handle,
         .subpass    = subpass,
         //
         .basePipelineHandle = VK_NULL_HANDLE,
         .basePipelineIndex = -1,
      };
      if (vkCreateGraphicsPipelines(this->owner->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->pipeline.handle) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create graphics pipeline.");
      }
   }

   void material::teardown_handle() {
      assert(this->owner);
      if (this->pipeline.handle != VK_NULL_HANDLE) {
         vkDestroyPipeline(this->owner->logical_device, this->pipeline.handle, nullptr);
         this->pipeline.handle = VK_NULL_HANDLE;
      }
   }
   #pragma endregion
}