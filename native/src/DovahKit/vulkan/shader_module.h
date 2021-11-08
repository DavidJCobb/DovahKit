#pragma once
#include <QByteArray>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class device;

   class shader_module : no_copy {
      public:
         shader_module(device&, const QByteArray&);
         ~shader_module();
      
         device*        owner  = nullptr;
         VkShaderModule handle = VK_NULL_HANDLE;

         shader_module(shader_module&&) noexcept;
         shader_module& operator=(shader_module&&) noexcept;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }
   };
}