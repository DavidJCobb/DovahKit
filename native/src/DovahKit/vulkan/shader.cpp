#include "shader.h"
#include <stdexcept>
#include "render_pass.h"
#include "surface_renderer.h"

namespace vulkanDK {
   shader::~shader() {
      if (auto*& p = this->config.area_override) {
         delete p;
         p = nullptr;
      }
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
      this->material.setup_handle(this->definition, viewport, scissor, this->config.render_pass->handle, this->config.subpass);
      if (auto* sr = this->material.owner) {
         std::string name;
         name.resize(8);
         for (size_t i = 0; i < 8; ++i)
            name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
         //
         sr->set_debug_object_name((uint64_t)this->material.pipeline.handle, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
      }
   }

   void shader::pre_resize() {
      this->material.teardown_handle();
   }
   void shader::post_resize(VkExtent2D view) {
      this->setup_pipeline(view);
   }
}