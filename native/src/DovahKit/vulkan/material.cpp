#include "material.h"
#include <stdexcept>

namespace DovahKit::vulkan {
   shader_module::shader_module(device& d, const QByteArray& comp) : owner(d), compiled(comp) {
      auto create_info = VkShaderModuleCreateInfo{
         .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = (uint32_t)this->compiled.size(),
         .pCode    = (const uint32_t*)this->compiled.data(),
      };
      if (vkCreateShaderModule(this->owner.logical, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKit::vulkan::shader_module::shader_module] Failed to create shader module.");
      }
   }
   shader_module::~shader_module() {
      vkDestroyShaderModule(this->owner.logical, this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
   }


   material::material(device& d) : owner(d) {
   }
   material::~material() {
   }

   void material::add_stage(const stage_info& s) {
      this->stages.push_back(s);
   }

   std::vector<VkPipelineShaderStageCreateInfo> material::stage_create_info() const {
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
}