#include "shader_module.h"
#include "helpers/byteswap.h"
#include "helpers/generic_reader.h"
#include "exceptions.h"
#include "spir_v_parser.h"

namespace vulkanDK {
   shader_module::shader_module(VkDevice d, const QByteArray& compiled) : device(d) {
      if (compiled.isNull())
         return;
      auto create_info = VkShaderModuleCreateInfo{
         .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = (uint32_t)compiled.size(),
         .pCode    = (const uint32_t*)compiled.data(),
      };
      this->device = d;
      if (auto result = vkCreateShaderModule(d, &create_info, nullptr, &this->handle); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::shader_module] Failed to create shader module.");
      }
      //
      auto parser = spir_v_parser();
      parser.read(compiled);
      if (parser.valid) {
         std::swap(this->data.entry_points, parser.entry_points);
         this->data.stages = parser.stages;
      }
   }
   shader_module::~shader_module() {
      if (this->device != VK_NULL_HANDLE && this->handle != VK_NULL_HANDLE)
         vkDestroyShaderModule(this->device, this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
      this->device = VK_NULL_HANDLE;
   }
   //
   shader_module::shader_module(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->device, o.device);
      std::swap(this->data,   o.data);
   }
   shader_module& shader_module::operator=(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->device, o.device);
      std::swap(this->data,   o.data);
      return *this;
   }

   const spir_v_entry_point* shader_module::entry_point(const std::string& name, VkShaderStageFlags stage) const {
      for (auto& ep : this->data.entry_points) {
         if ((ep.stages & stage) == 0)
            continue;
         if (ep.name == name)
            return &ep;
      }
      return nullptr;
   }
}