#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "shader_module.h"

namespace vulkanDK {
   class surface_renderer;

   class material_definition {
      public:
         struct stage_info {
            shader_module* module = nullptr;
            const char*    entry_point_name = nullptr; // function in the shader to call
            VkPipelineShaderStageCreateFlags flags = {};
            VkShaderStageFlagBits            stage = {};
            const VkSpecializationInfo*      specialization_info = nullptr; // can pass parameters to the shader
         };

      public:
         std::vector<stage_info> stages;

         void add_stage(const stage_info&);

         std::vector<VkPipelineShaderStageCreateInfo> stage_create_info() const;
   };

   class material : no_copy {
      protected:
         material_definition* source = nullptr;
      public:
         material(surface_renderer&, material_definition*);
         ~material();

         material(material&&) noexcept;
         material& operator=(material&&) noexcept;

         surface_renderer* owner = nullptr;

         struct {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipeline       handle = VK_NULL_HANDLE;
         } pipeline;
   };
}