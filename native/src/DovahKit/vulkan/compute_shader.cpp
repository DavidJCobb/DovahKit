#include "compute_shader.h"
#include <stdexcept>
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
   compute_shader::~compute_shader() {
      if (!this->owner)
         return;
      if (this->pipeline.handle != VK_NULL_HANDLE) {
         vkDestroyPipeline(this->owner->logical_device, this->pipeline.handle, nullptr);
         this->pipeline.handle = VK_NULL_HANDLE;
      }
      if (this->pipeline.layout != VK_NULL_HANDLE) {
         vkDestroyPipelineLayout(this->owner->logical_device, this->pipeline.layout, nullptr);
         this->pipeline.layout = VK_NULL_HANDLE;
      }
   }

   void compute_shader::set_layout_info(const std::vector<VkDescriptorSetLayout>& dsl) {
      this->config.descriptor_set_layouts = dsl;
   }

   void compute_shader::setup(surface_renderer& owner) {
      this->owner = &owner;
      #if _DEBUG
         qDebug("[vulkanDK::compute_shader::setup_pipeline] Setting up: %s ...", _id_to_string(this->id).c_str());
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
         owner.set_debug_object_name(this->pipeline.layout, _id_to_string(this->id));
      }
      //
      #if _DEBUG
         if (auto* sm = this->config.stage.module) {
            const auto& name = this->config.stage.entry_point_name;
            bool found = false;
            for (auto& ep : sm->data.entry_points) {
               if (ep.name == name) {
                  if (!(ep.stages & VK_SHADER_STAGE_COMPUTE_BIT))
                     throw std::logic_error("[vulkanDK::compute_shader::setup] The shader_module does not appear to have a compute shader entry point.");
                  found = true;
                  break;
               }
            }
            if (!found)
               throw std::logic_error("[vulkanDK::compute_shader::setup] The shader_module does not appear to have an entry point with the specified name.");
         } else {
            throw std::logic_error("[vulkanDK::compute_shader::setup] The shader_module is missing.");
         }
      #endif
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
         sr->set_debug_object_name(this->pipeline.handle, _id_to_string(this->id));
      }
   }
}