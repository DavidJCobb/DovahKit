#pragma once
#include "../_vulkan.h"

namespace vulkanDK {
   struct bundled_descriptor_buffer_write {
      VkDescriptorBufferInfo info;
      VkWriteDescriptorSet   write;
   };
}