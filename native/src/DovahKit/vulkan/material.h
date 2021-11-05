#pragma once
#include <vector>
#include <QByteArray>
#include "device.h"

namespace DovahKit::vulkan {
   class shader_module {
      protected:
         device& owner;
      public:
         QByteArray     compiled;
         VkShaderModule handle = VK_NULL_HANDLE;

         shader_module(device&, const QByteArray&);
         ~shader_module();
   };

   class material {
      public:
         struct stage_info {
            shader_module* module = nullptr;
            const char*    entry_point_name = nullptr; // function in the shader to call
            VkPipelineShaderStageCreateFlags flags = {};
            VkShaderStageFlagBits            stage = {};
            const VkSpecializationInfo*      specialization_info = nullptr; // can pass parameters to the shader
         };

      protected:
         device& owner;
      public:
         material(device&);
         ~material();

         std::vector<stage_info> stages;

         void add_stage(const stage_info&);

         std::vector<VkPipelineShaderStageCreateInfo> stage_create_info() const;

         inline const VkDevice logical_device() const noexcept { return this->owner.logical; }
   };

   class material_per_swap_chain {
      protected:
         material& source;
      public:
         material_per_swap_chain(material&);
         ~material_per_swap_chain();

         struct {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipeline       handle = VK_NULL_HANDLE;
         } pipeline;
   };
}