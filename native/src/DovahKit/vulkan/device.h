#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"

namespace vulkanDK {
   class device : no_copy {
      public:
         device(VkPhysicalDevice);
         ~device();
      
         VkDevice         logical  = VK_NULL_HANDLE;
         VkPhysicalDevice physical = VK_NULL_HANDLE;
         struct {
            bool  null_descriptors      = false;
            float anisotropic_filtering = 0; // max
         } support;
         struct {
            VkQueue graphics     = VK_NULL_HANDLE;
            VkQueue presentation = VK_NULL_HANDLE;
         } queues;

         VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

         VkFormat find_depth_format() const;

         uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags) const;

         buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);
   };
}