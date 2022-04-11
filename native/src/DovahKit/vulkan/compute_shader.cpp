#include "compute_shader.h"
#include <stdexcept>
#include "exceptions.h"
#include "render_pass.h"
#include "surface_renderer.h"

namespace vulkanDK {
   compute_shader::~compute_shader() {
   }

   void compute_shader::set_layout_info(const std::vector<VkDescriptorSetLayout>& dsl) {
      this->config.descriptor_set_layouts = dsl;
   }

   void compute_shader::setup(surface_renderer& owner) {
      this->owner = &owner;
      #if _DEBUG
      {
         std::string name;
         name.resize(8);
         for (size_t i = 0; i < 8; ++i)
            name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
         //
         qDebug("[vulkanDK::compute_shader::setup_pipeline] Setting up: %s ...", name.c_str());
      }
      #endif
      //
      {  // Pipeline layout
         auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = (uint32_t)this->config.descriptor_set_layouts.size(),
            .pSetLayouts            = this->config.descriptor_set_layouts.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
         };
         if (auto result = vkCreatePipelineLayout(owner.logical_device, &pipeline_layout_info, nullptr, &this->pipeline.layout); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::compute_shader::setup] Failed to create pipeline layout.");
         }
         //
         {
            std::string name;
            name.resize(8);
            for (size_t i = 0; i < 8; ++i)
               name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
            //
            owner.set_debug_object_name(this->pipeline.layout, name);
         }
      }
      //
      auto create_info = VkComputePipelineCreateInfo{
         .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
         .pNext = nullptr,
         .stage = {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext  = nullptr,
            .flags  = 0,
            .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = this->config.stage.module->handle,
            .pName  = this->config.stage.entry_point_name,
         },
         .layout = this->pipeline.layout,
         .basePipelineHandle = VK_NULL_HANDLE,
         .basePipelineIndex  = 0,
      };
      vkCreateComputePipelines(
         owner.logical_device,
         VK_NULL_HANDLE,
         1,
         &create_info,
         nullptr,
         &this->pipeline.handle
      );
      if (auto* sr = this->get_owner()) {
         std::string name;
         name.resize(8);
         for (size_t i = 0; i < 8; ++i)
            name[i] = (this->id.value >> (i * 0x8)) & 0xFF;
         //
         sr->set_debug_object_name(this->pipeline.handle, name);
      }
   }
}