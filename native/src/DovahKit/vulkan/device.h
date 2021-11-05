#pragma once
#include <vector>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace DovahKit::vulkan {
   class device {
      public:
         VkPhysicalDevice physical = VK_NULL_HANDLE;
         VkDevice         logical  = VK_NULL_HANDLE;

      public:
         device(VkPhysicalDevice);
         ~device();

         VkImageView create_image_view(VkImage, VkFormat, VkImageAspectFlags) const;
         VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

         VkFormat find_depth_format() const;
   };
}