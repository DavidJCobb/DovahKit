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
   #pragma endregion

   #pragma region material
   material::material(surface_renderer& c, material_definition* d) : source(d), owner(&c) {
   }
   material::~material() {
      auto device = this->owner->device.handle;
      vkDestroyPipeline      (device, this->pipeline.handle, nullptr);
      vkDestroyPipelineLayout(device, this->pipeline.layout, nullptr);
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
   #pragma endregion
}