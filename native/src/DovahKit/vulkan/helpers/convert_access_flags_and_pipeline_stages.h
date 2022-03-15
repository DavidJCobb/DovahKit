#pragma once
#include "../_vulkan.h"

namespace vulkanDK {
   extern VkPipelineStageFlags access_flags_to_pipeline_stages(VkAccessFlags, VkPipelineStageFlags allowed_shaders);
}