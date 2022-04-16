#pragma once
#include <vector>
#include <QByteArray>
#include <QString>
#include "_vulkan.h"
#include "_util.h"
#include "spir_v_entry_point.h"

namespace vulkanDK {
   class shader_module : no_copy {
      public:
         shader_module(VkDevice, const QByteArray&);
         ~shader_module();
      
         VkDevice       device = VK_NULL_HANDLE;
         VkShaderModule handle = VK_NULL_HANDLE;
         struct {
            QString name;
            VkShaderStageFlags stages = VK_SHADER_STAGE_ALL;
            std::vector<spir_v_entry_point> entry_points;
         } data;

         shader_module(shader_module&&) noexcept;
         shader_module& operator=(shader_module&&) noexcept;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }

         const spir_v_entry_point* entry_point(const std::string& name, VkShaderStageFlags stage) const;
   };
}