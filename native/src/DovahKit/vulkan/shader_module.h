#pragma once
#include <QByteArray>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class shader_module : no_copy {
      public:
         shader_module(VkDevice, const QByteArray&);
         ~shader_module();
      
         VkDevice        device = VK_NULL_HANDLE;
         VkShaderModule  handle = VK_NULL_HANDLE;

         shader_module(shader_module&&) noexcept;
         shader_module& operator=(shader_module&&) noexcept;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }
   };
}