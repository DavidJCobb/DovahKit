#pragma once
#include <QByteArray>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class logical_device;

   class shader_module : no_copy {
      public:
         shader_module(logical_device&, const QByteArray&);
         ~shader_module();
      
         logical_device* device = nullptr;
         VkShaderModule  handle = VK_NULL_HANDLE;

         shader_module(shader_module&&) noexcept;
         shader_module& operator=(shader_module&&) noexcept;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }
   };
}