#include "shader_module.h"
#include <stdexcept>
#include "device.h"

namespace vulkanDK {
   shader_module::shader_module(device& d, const QByteArray& compiled) : owner(&d) {
      if (compiled.isNull())
         return;
      auto create_info = VkShaderModuleCreateInfo{
         .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = (uint32_t)compiled.size(),
         .pCode    = (const uint32_t*)compiled.data(),
      };
      if (vkCreateShaderModule(d.logical, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::shader_module::shader_module] Failed to create shader module.");
      }
   }
   shader_module::~shader_module() {
      if (this->owner != nullptr && this->handle != VK_NULL_HANDLE)
         vkDestroyShaderModule(this->owner->logical, this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
      this->owner  = nullptr;
   }
   //
   shader_module::shader_module(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->owner,  o.owner);
   }
   shader_module& shader_module::operator=(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->owner,  o.owner);
      return *this;
   }
}