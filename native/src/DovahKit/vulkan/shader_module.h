#pragma once
#include <vector>
#include <QByteArray>
#include <QString>
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class shader_module : no_copy {
      public:
         struct entry_point {
            std::string name;
            VkShaderStageFlags stages = 0;
         };

      public:
         shader_module(VkDevice, const QByteArray&);
         ~shader_module();
      
         VkDevice       device = VK_NULL_HANDLE;
         VkShaderModule handle = VK_NULL_HANDLE;
         struct {
            QString name;
            VkShaderStageFlags stages = VK_SHADER_STAGE_ALL;
            std::vector<entry_point> entry_points;
         } data;

         shader_module(shader_module&&) noexcept;
         shader_module& operator=(shader_module&&) noexcept;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }
   };
}