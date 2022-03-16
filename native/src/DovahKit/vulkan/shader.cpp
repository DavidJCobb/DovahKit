#include "shader.h"
#include <stdexcept>
#include "material.h"
#include "render_pass.h"
#include "surface_renderer.h"

namespace vulkanDK {
   #pragma region shader::variant
   void shader::variant::setup(shader& owner, VkViewport viewport, VkRect2D scissor, render_pass& render_pass, uint32_t subpass) {
      const auto& def = owner.definition;
      //
      auto stages = def.stage_create_info();
      auto depth  = def.depth_stencil_info();
      auto vertex = def.vertex_info();
      //
      auto blends = def.color_blend_attachment_info();
      auto color  = def.color_blend_info(blends);
      //
      auto dyn    = def.dynamic_state_info();

      auto viewport_create = VkPipelineViewportStateCreateInfo{
         .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .pNext         = nullptr,
         .flags         = 0,
         .viewportCount = 1,
         .pViewports    = &viewport,
         .scissorCount  = 1,
         .pScissors     = &scissor,
      };

      auto rasterization = def.rasterization;
      if (this->face_cull_mode.has_value())
         rasterization.cullMode = this->face_cull_mode.value();

      auto pipeline_info = VkGraphicsPipelineCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
         .stageCount = (uint32_t)stages.stages.size(),
         .pStages    = stages.stages.data(),
         //
         .pVertexInputState   = &vertex,
         .pInputAssemblyState = &def.inputs.triangles,
         .pViewportState      = &viewport_create,
         .pRasterizationState = &rasterization,
         .pMultisampleState   = &def.multisampling,
         .pDepthStencilState  = &depth,
         .pColorBlendState    = &color,
         .pDynamicState       = &dyn,
         //
         .layout = owner.material.pipeline.layout,
         //
         .renderPass = render_pass.handle,
         .subpass    = subpass,
         //
         .basePipelineHandle = VK_NULL_HANDLE,
         .basePipelineIndex = -1,
      };
      if (vkCreateGraphicsPipelines(owner.get_owner()->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::material::setup_handle] Failed to create graphics pipeline.");
      }
   }
   void shader::variant::teardown(shader& owner) {
      if (this->handle != VK_NULL_HANDLE) {
         vkDestroyPipeline(owner.get_owner()->logical_device, this->handle, nullptr);
         this->handle = VK_NULL_HANDLE;
      }
   }
   #pragma endregion

   shader::~shader() {
      if (auto*& p = this->config.area_override) {
         delete p;
         p = nullptr;
      }
      for (auto& item : this->variants)
         item.teardown(*this);
   }

   surface_renderer* shader::get_owner() const {
      return this->material.owner;
   }

   void shader::set_render_pass(render_pass* rp, uint32_t subpass) {
      this->config.render_pass = rp;
      this->config.subpass     = subpass;
   }
   void shader::set_area_override_info(const area_override_data& aod) {
      auto& ptr = this->config.area_override;
      if (!ptr)
         ptr = new area_override_data;
      *ptr = aod;
   }
   void shader::set_layout_info(const std::vector<VkDescriptorSetLayout>& dsl, const std::vector<VkPushConstantRange>& pcr) {
      this->config.descriptor_set_layouts = dsl;
      this->config.push_constant_ranges   = pcr;
   }

   void shader::setup_pipeline_layout(surface_renderer& sr) {
      this->material.owner = &sr;
      this->material.setup_layout(this->config.descriptor_set_layouts, this->config.push_constant_ranges);
      //
      {
         std::string name;
         name.resize(8);
         for (size_t i = 0; i < 8; ++i)
            name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
         //
         sr.set_debug_object_name((uint64_t)this->material.pipeline.layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
      }
   }
   void shader::setup_pipeline(VkExtent2D view) {
      if (!this->config.render_pass) {
         throw std::runtime_error("[vulkanDK::shader::setup_pipeline] No render pass set for this shader.");
      }
      #if _DEBUG
      {
         std::string name;
         name.resize(8);
         for (size_t i = 0; i < 8; ++i)
            name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
         //
         qDebug("[vulkanDK::shader::setup_pipeline] Setting up: %s ...", name.c_str());
      }
      #endif
      //
      auto viewport = VkViewport{ // describe what part of the framebuffer we should draw to
         .x        = 0.0,
         .y        = 0.0,
         .width    = (float)view.width,
         .height   = (float)view.height,
         .minDepth = 0.0, // must be >= 0
         .maxDepth = 1.0, // must be <= 1
      };
      auto scissor = VkRect2D{ // describe what part of the framebuffer we should retain (like a write-mask)
         .offset = {0, 0},
         .extent = view,
      };
      if (auto* ao = this->config.area_override) {
         if (ao->handler)
            (ao->handler)(*ao, view);
         viewport = ao->viewport;
         scissor  = ao->scissor;
      }
      this->material.setup_handle(this->definition, viewport, scissor, *this->config.render_pass, this->config.subpass);
      if (auto* sr = this->get_owner()) {
         std::string name;
         name.resize(8);
         for (size_t i = 0; i < 8; ++i)
            name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
         //
         sr->set_debug_object_name((uint64_t)this->material.pipeline.handle, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
      }
      //
      for (auto& item : this->variants) {
         item.setup(*this, viewport, scissor, *this->config.render_pass, this->config.subpass);
      }
   }

   void shader::add_variant(const variant_definition& dfn) {
      this->variants.push_back(variant(dfn));
   }
   const shader::variant* shader::get_variant(const variant_definition& dfn) const {
      for (const auto& item : this->variants)
         if (item == dfn)
            return &item;
      return nullptr;
   }

   void shader::pre_resize() {
      this->material.teardown_handle();
      for (auto& item : this->variants)
         item.teardown(*this);
   }
   void shader::post_resize(VkExtent2D view) {
      this->setup_pipeline(view);
   }
}