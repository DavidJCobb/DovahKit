#include "material.h"
#include <stdexcept>
#include "logical_device.h"
#include "surface_renderer.h"

namespace vulkanDK {
   #pragma region material_definition
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
   #pragma endregion

   #pragma region material
   material::material(surface_renderer& c) : owner(&c) {
   }
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

      auto pipeline_info = VkGraphicsPipelineCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
         .stageCount = (uint32_t)stages.size(),
         .pStages    = stages.data(), 
         //
         .pVertexInputState   = &def.inputs.vertex,
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