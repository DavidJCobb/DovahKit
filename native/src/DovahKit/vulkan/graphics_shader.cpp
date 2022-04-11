#include "graphics_shader.h"
#include "config/is_righthanded.h"
#include "exceptions.h"
#include "render_pass.h"
#include "surface_renderer.h"

namespace {
   std::string _id_to_string(const cobb::eight_cc id) {
      std::string name;
      name.resize(8);
      for (size_t i = 0; i < 8; ++i)
         name[i] = id.bytes[i];
      return name;
   }
}

namespace vulkanDK {
   #pragma region graphics_shader::variant
   void graphics_shader::variant::setup(graphics_shader& owner, VkViewport viewport, VkRect2D scissor, render_pass& render_pass, uint32_t subpass) {
      auto stages = owner.stage_create_info();
      auto depth  = owner.depth_stencil_info();
      auto vertex = owner.vertex_info();
      //
      auto blends = owner.color_blend_attachment_info();
      auto color  = owner.color_blend_info(blends);
      //
      auto dyn    = owner.dynamic_state_info();

      auto viewport_create = VkPipelineViewportStateCreateInfo{
         .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .pNext         = nullptr,
         .flags         = 0,
         .viewportCount = 1,
         .pViewports    = &viewport,
         .scissorCount  = 1,
         .pScissors     = &scissor,
      };

      auto rasterization = owner.options.rasterization;
      if (this->face_cull_mode.has_value())
         rasterization.cullMode = this->face_cull_mode.value();

      auto pipeline_info = VkGraphicsPipelineCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
         .stageCount = (uint32_t)stages.stages.size(),
         .pStages    = stages.stages.data(),
         //
         .pVertexInputState   = &vertex,
         .pInputAssemblyState = &owner.options.inputs.triangles,
         .pViewportState      = &viewport_create,
         .pRasterizationState = &rasterization,
         .pMultisampleState   = &owner.options.multisampling,
         .pDepthStencilState  = &depth,
         .pColorBlendState    = &color,
         .pDynamicState       = &dyn,
         //
         .layout = owner.pipeline.layout,
         //
         .renderPass = render_pass.handle,
         .subpass    = subpass,
         //
         .basePipelineHandle = VK_NULL_HANDLE,
         .basePipelineIndex = -1,
      };
      if (auto result = vkCreateGraphicsPipelines(owner.get_owner()->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->handle); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::material::setup_handle] Failed to create graphics pipeline.");
      }
   }
   void graphics_shader::variant::teardown(graphics_shader& owner) {
      if (this->handle != VK_NULL_HANDLE) {
         vkDestroyPipeline(owner.get_owner()->logical_device, this->handle, nullptr);
         this->handle = VK_NULL_HANDLE;
      }
   }
   #pragma endregion

   graphics_shader::graphics_shader(surface_renderer& sr) : _owner(&sr) {
      this->options.inputs.triangles = VkPipelineInputAssemblyStateCreateInfo{
         .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .pNext                  = nullptr,
         .flags                  = 0,
         .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
         .primitiveRestartEnable = VK_FALSE,
      };
      this->options.multisampling = VkPipelineMultisampleStateCreateInfo{ // MSAA (multisampling anti-alias); disable it by default (one sample only)
         .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
         .sampleShadingEnable   = VK_FALSE,
         .minSampleShading      = 1.0,
         .pSampleMask           = nullptr,
         .alphaToCoverageEnable = VK_FALSE,
         .alphaToOneEnable      = VK_FALSE,
      };
      this->options.rasterization = VkPipelineRasterizationStateCreateInfo{
         .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
         .depthClampEnable        = VK_FALSE,
         .rasterizerDiscardEnable = VK_FALSE, // setting this to true basically disables the rasterizer entirely
         .polygonMode             = VK_POLYGON_MODE_FILL,  // fill polygons, or render wireframes or point clouds?
         .cullMode                = VK_CULL_MODE_BACK_BIT, // cull backfaces, frontfaces, or no faces
         .frontFace               = (config::is_righthanded) ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE, // specify which vertex order (clockwise or counterclockwise) signifies a face pointing toward us
         .depthBiasEnable         = VK_FALSE,
         .depthBiasConstantFactor = 0.0,
         .depthBiasClamp          = 0.0,
         .depthBiasSlopeFactor    = 0.0,
         .lineWidth               = 1.0, // line width, e.g. for wireframes; must be 1.0 unless the device supports wide lines and you enable that feature
      };
   }
   graphics_shader::graphics_shader(graphics_shader&& other) {
      std::swap(this->id,        other.id);
      std::swap(this->on_resize, other.on_resize);
      std::swap(this->options,   other.options);
      std::swap(this->pipeline,  other.pipeline);
      std::swap(this->variants,  other.variants);
   }
   graphics_shader::~graphics_shader() {
      if (!this->_owner)
         return;
      for (auto& v : this->variants)
         v.teardown(*this);
      this->teardown_pipeline();
      if (this->pipeline.layout != VK_NULL_HANDLE) {
         vkDestroyPipelineLayout(this->_owner->logical_device, this->pipeline.layout, nullptr);
         this->pipeline.layout = VK_NULL_HANDLE;
      }
   }

   void graphics_shader::add_variant(const variant_definition& dfn) {
      this->variants.push_back(variant(dfn));
   }
   [[nodiscard]] const graphics_shader::variant* graphics_shader::get_variant(const variant_definition& dfn) const {
      for (const auto& item : this->variants)
         if (item == dfn)
            return &item;
      return nullptr;
   }

   void graphics_shader::add_stage(const pipeline_stage_info& s) {
      this->options.stages.push_back(s);
   }
   void graphics_shader::set_layout_info(const std::vector<VkDescriptorSetLayout>& dsl, const std::vector<VkPushConstantRange>& pcr) {
      this->options.descriptor_set_layouts = dsl;
      this->options.push_constant_ranges   = pcr;
   }
   void graphics_shader::set_render_pass(render_pass* rp, uint32_t subpass) {
      this->options.when.render_pass = rp;
      this->options.when.subpass     = subpass;
   }
   //
   void graphics_shader::setup_pipeline_layout() {
      auto& dsl = this->options.descriptor_set_layouts;
      auto& pcr = this->options.push_constant_ranges;
      auto  pipeline_layout_info = VkPipelineLayoutCreateInfo{
         .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount         = (uint32_t)dsl.size(),
         .pSetLayouts            = dsl.data(),
         .pushConstantRangeCount = (uint32_t)pcr.size(),
         .pPushConstantRanges    = pcr.data(),
      };
      if (auto result = vkCreatePipelineLayout(this->get_owner()->logical_device, &pipeline_layout_info, nullptr, &this->pipeline.layout); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::graphics_shader::setup_pipeline_layout] vkCreatePipelineLayout failed.");
      }
      //
      this->get_owner()->set_debug_object_name(this->pipeline.layout, _id_to_string(this->id));
   }
   void graphics_shader::setup_pipeline(VkExtent2D surface) {
      if (!this->options.when.render_pass) {
         throw std::logic_error("[vulkanDK::graphics_shader::setup_pipeline] No render pass set for this shader.");
      }
      #if _DEBUG
         qDebug("[vulkanDK::graphics_shader::setup_pipeline] Setting up: %s ...", _id_to_string(this->id).c_str());
      #endif
      //
      if (this->options.area.mode == area_mode::whole_surface) {
         this->options.area.scissor = VkRect2D{ // describe what part of the framebuffer we should retain (like a write-mask)
            .offset = {0, 0},
            .extent = surface,
         };
         this->options.area.viewport = VkViewport{ // describe what part of the framebuffer we should draw to
            .x        = 0.0,
            .y        = 0.0,
            .width    = (float)surface.width,
            .height   = (float)surface.height,
            .minDepth = 0.0, // must be >= 0
            .maxDepth = 1.0, // must be <= 1
         };
      }
      //
      if (this->pipeline.layout == VK_NULL_HANDLE) {
         throw std::logic_error("[vulkanDK::graphics_shader::setup_pipeline] You must set up the pipeline layout first.");
      }
      auto stages = this->stage_create_info();
      auto depth  = this->depth_stencil_info();
      auto vertex = this->vertex_info();
      //
      auto blends = this->color_blend_attachment_info();
      auto color  = this->color_blend_info(blends);
      //
      auto dyn    = this->dynamic_state_info();

      auto viewport_create = VkPipelineViewportStateCreateInfo{
         .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .pNext         = nullptr,
         .flags         = 0,
         .viewportCount = 1,
         .pViewports    = &this->options.area.viewport,
         .scissorCount  = 1,
         .pScissors     = &this->options.area.scissor,
      };

      {  // Simple validation for render passes.
         if (this->options.when.subpass >= this->options.when.render_pass->subpasses.size()) {
            throw vuid_exception(06046, "subpass index out of range");
         }
         //
         const auto& rp_subpass = this->options.when.render_pass->subpasses[this->options.when.subpass];
         if (!rp_subpass.attachments.color.empty()) {
            if (blends.size() != rp_subpass.attachments.color.size()) {
               throw vuid_exception(06042, "mismatch between pipeline color blend count and render pass color attachment count");
            }
         }
      }

      auto pipeline_info = VkGraphicsPipelineCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
         .stageCount = (uint32_t)stages.stages.size(),
         .pStages    = stages.stages.data(),
         //
         .pVertexInputState   = &vertex,
         .pInputAssemblyState = &this->options.inputs.triangles,
         .pViewportState      = &viewport_create,
         .pRasterizationState = &this->options.rasterization,
         .pMultisampleState   = &this->options.multisampling,
         .pDepthStencilState  = &depth,
         .pColorBlendState    = &color,
         .pDynamicState       = &dyn,
         //
         .layout = this->pipeline.layout,
         //
         .renderPass = this->options.when.render_pass->handle,
         .subpass    = this->options.when.subpass,
         //
         .basePipelineHandle = VK_NULL_HANDLE,
         .basePipelineIndex = -1,
      };
      if (auto result = vkCreateGraphicsPipelines(this->get_owner()->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->pipeline.handle); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::graphics_shader::setup_pipeline] Failed to create graphics pipeline.");
      }
      //
      if (auto* sr = this->get_owner()) {
         sr->set_debug_object_name(this->pipeline.handle, _id_to_string(this->id));
      }
      //
      for (auto& item : this->variants) {
         item.setup(*this, this->options.area.viewport, this->options.area.scissor, *this->options.when.render_pass, this->options.when.subpass);
      }
   }

   void graphics_shader::teardown_pipeline() {
      if (this->pipeline.handle != VK_NULL_HANDLE) {
         vkDestroyPipeline(this->get_owner()->logical_device, this->pipeline.handle, nullptr);
         this->pipeline.handle = VK_NULL_HANDLE;
      }
   }

   void graphics_shader::pre_resize() {
      this->teardown_pipeline();
      for (auto& item : this->variants)
         item.teardown(*this);
   }
   void graphics_shader::post_resize(VkExtent2D extent) {
      if (this->on_resize)
         (this->on_resize)(*this, extent);
      this->setup_pipeline(extent);
   }

   #pragma region options to Vulkan structs
   graphics_shader::bundled_stage_create_info graphics_shader::stage_create_info() const {
      bundled_stage_create_info out;
      //
      auto& list = this->options.stages;
      auto  size = list.size();
      out.stages.resize(size);
      for (const auto& item : list) {
         const auto& info = item.specialization_info;
         if (info.empty())
            continue;
         out.specializations.push_back(VkSpecializationInfo{
            .mapEntryCount = (uint32_t)info.fields.size(),
            .pMapEntries   = info.fields.data(),
            .dataSize      = (uint32_t)info.data.size(),
            .pData         = info.data.data(),
         });
      }
      //
      size_t spec = 0;
      for (size_t i = 0; i < size; ++i) {
         auto& stage = list[i];
         auto& item  = out.stages[i];
         item = VkPipelineShaderStageCreateInfo{
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = stage.stage,
            .module = stage.module->handle,
            .pName  = stage.entry_point_name,
            .pSpecializationInfo = nullptr,
         };
         if (!stage.specialization_info.empty()) {
            item.pSpecializationInfo = &out.specializations[spec];
            ++spec;
         }
      }
      //
      return out;
   }

   VkPipelineDepthStencilStateCreateInfo graphics_shader::depth_stencil_info() const {
      return VkPipelineDepthStencilStateCreateInfo{
         .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
         .depthTestEnable       = this->options.depth.testing,
         .depthWriteEnable      = this->options.depth.writing,
         .depthCompareOp        = this->options.depth.comparison,
         .depthBoundsTestEnable = this->options.depth.culling.enabled,
         .stencilTestEnable     = this->options.stencil.testing,
         .front                 = this->options.stencil.front,
         .back                  = this->options.stencil.back,
         .minDepthBounds        = this->options.depth.culling.min,
         .maxDepthBounds        = this->options.depth.culling.max,
      };
   }

   std::vector<VkPipelineColorBlendAttachmentState> graphics_shader::color_blend_attachment_info() const {
      std::vector<VkPipelineColorBlendAttachmentState> out;
      auto& list = this->options.color_blending.blends;
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
   VkPipelineColorBlendStateCreateInfo graphics_shader::color_blend_info(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) const {
      return VkPipelineColorBlendStateCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
         .logicOpEnable   = VK_FALSE,         // Enables bitwise-operation blending. Mutually exclusive with "attachment state" 
         .logicOp         = VK_LOGIC_OP_COPY, // blending and will disable that.
         .attachmentCount = (uint32_t)attachments.size(),
         .pAttachments    = attachments.data(),
         .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
      };
   }

   VkPipelineVertexInputStateCreateInfo graphics_shader::vertex_info() const {
      auto& src = this->options.inputs.vertex;
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

   VkPipelineDynamicStateCreateInfo graphics_shader::dynamic_state_info() const {
      return VkPipelineDynamicStateCreateInfo{
         .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
         .pNext = nullptr,
         .flags = 0,
         .dynamicStateCount = (uint32_t)this->options.dynamic_states.size(),
         .pDynamicStates    = this->options.dynamic_states.data(),
      };
   }
   #pragma endregion
}