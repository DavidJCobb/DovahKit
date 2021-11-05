#include "device.h"
#include <stdexcept>

namespace DovahKit::vulkan {
   VkImageView device::create_image_view(VkImage, VkFormat, VkImageAspectFlags) const;
   VkFormat device::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
      for (VkFormat format : candidates) {
         VkFormatProperties props;
         vkGetPhysicalDeviceFormatProperties(this->physical, format, &props);
         //
         if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
         } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
         }
      }
      return VK_FORMAT_UNDEFINED;
   }

   VkFormat device::find_depth_format() const {
      auto fmt = this->find_supported_format(
         { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
         VK_IMAGE_TILING_OPTIMAL,
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
      );
      if (fmt == VK_FORMAT_UNDEFINED) {
         throw std::runtime_error("[DovahKit::vulkan::device::find_depth_format] No format.");
      }
      return fmt;
   }
}