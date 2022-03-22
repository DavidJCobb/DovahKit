#include "shader_module.h"
#include "exceptions.h"

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
   }
   shader_module& shader_module::operator=(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->device, o.device);
      return *this;
   }
}