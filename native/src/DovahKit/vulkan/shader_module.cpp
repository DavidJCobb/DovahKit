#include "shader_module.h"
#include <stdexcept>
#include "logical_device.h"

namespace vulkanDK {
   shader_module::shader_module(logical_device& d, const QByteArray& compiled) : device(&d) {
      if (compiled.isNull())
         return;
      auto create_info = VkShaderModuleCreateInfo{
         .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = (uint32_t)compiled.size(),
         .pCode    = (const uint32_t*)compiled.data(),
      };
      if (vkCreateShaderModule(d.handle, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         this->handle = VK_NULL_HANDLE;
         return;
      }
   }
   shader_module::~shader_module() {
      if (this->device != nullptr && this->handle != VK_NULL_HANDLE)
         vkDestroyShaderModule(this->device->handle, this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
      this->device = nullptr;
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