#include "device.h"
#include <stdexcept>

namespace vulkanDK {
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
         throw std::runtime_error("[vulkanDK::device::find_depth_format] No format.");
      }
      return fmt;
   }
   uint32_t device::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(this->physical, &memProperties);
      //
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
         if ((typeFilter & (1 << i)) == 0)
            continue;
         if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
         }
      }
      throw std::runtime_error("[vulkanDK::device::device::find_memory_type] Failed to find suitable memory type.");
   }

   buffer device::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
      buffer out = buffer(*this);
      //
      auto buffer_info = VkBufferCreateInfo{
         .sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size         = size,
         .usage        = usage,
         .sharingMode  = VK_SHARING_MODE_EXCLUSIVE,
      };
      if (vkCreateBuffer(this->logical, &buffer_info, nullptr, &out.handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::device::create_buffer] Failed to create vertex buffer.");
      }
      //
      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(this->logical, out.handle, &memRequirements);
      out.size = memRequirements.size;
      //
      // In a real-world application, you wouldn't use vkAllocateMemory for each individual object you wish 
      // to render, because there's actually a limit on the number of allocations you can make irrespective 
      // of their total size. Even on high-end hardware, that limit may be in the low thousands, the Vulkan 
      // tutorial gives 4096 as a plausible limit for  hardware like an NVIDIA GTX 1080. What you'd want to 
      // do instead, then, is allocate memory in larger blocks and then manually divide those blocks up for 
      // different objects -- similar to what you'd do when making a block allocator.
      //
      auto alloc_info = VkMemoryAllocateInfo{
         .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize  = memRequirements.size,
         .memoryTypeIndex = this->find_memory_type(memRequirements.memoryTypeBits, properties),
      };
      if (vkAllocateMemory(this->logical, &alloc_info, nullptr, &out.memory) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::device::create_buffer] Failed to allocate vertex buffer memory.");
      }

      vkBindBufferMemory(this->logical, out.handle, out.memory, 0);
   }
}