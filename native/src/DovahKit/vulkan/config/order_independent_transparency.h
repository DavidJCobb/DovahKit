#pragma once
#include "../_vulkan.h"

namespace vulkanDK::config {
   static constexpr VkFormat format_for_oit_accumulator = VK_FORMAT_R16G16B16A16_SFLOAT;
   static constexpr VkFormat format_for_oit_reveal      = VK_FORMAT_R16_SFLOAT;
}
