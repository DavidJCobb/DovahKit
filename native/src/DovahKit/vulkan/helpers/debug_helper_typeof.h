#pragma once
#include "../_vulkan.h"

namespace vulkanDK {
   namespace impl::debug_helper_typeof {

   }

   template<typename T> constexpr VkDebugReportObjectTypeEXT debug_helper_typeof = ([]() {
      if constexpr (std::is_same_v<T, VkInstance>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
      if constexpr (std::is_same_v<T, VkPhysicalDevice>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
      if constexpr (std::is_same_v<T, VkDevice>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
      if constexpr (std::is_same_v<T, VkQueue>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
      if constexpr (std::is_same_v<T, VkSemaphore>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT;
      if constexpr (std::is_same_v<T, VkCommandBuffer>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
      if constexpr (std::is_same_v<T, VkFence>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT;
      if constexpr (std::is_same_v<T, VkDeviceMemory>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
      if constexpr (std::is_same_v<T, VkBuffer>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
      if constexpr (std::is_same_v<T, VkImage>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
      if constexpr (std::is_same_v<T, VkEvent>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
      if constexpr (std::is_same_v<T, VkQueryPool>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT;
      if constexpr (std::is_same_v<T, VkBufferView>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT;
      if constexpr (std::is_same_v<T, VkImageView>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
      if constexpr (std::is_same_v<T, VkShaderModule>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
      if constexpr (std::is_same_v<T, VkPipelineCache>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT;
      if constexpr (std::is_same_v<T, VkPipelineLayout>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT;
      if constexpr (std::is_same_v<T, VkRenderPass>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT;
      if constexpr (std::is_same_v<T, VkPipeline>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT;
      if constexpr (std::is_same_v<T, VkDescriptorSetLayout>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
      if constexpr (std::is_same_v<T, VkSampler>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT;
      if constexpr (std::is_same_v<T, VkDescriptorPool>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT;
      if constexpr (std::is_same_v<T, VkDescriptorSet>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT;
      if constexpr (std::is_same_v<T, VkFramebuffer>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT;
      if constexpr (std::is_same_v<T, VkCommandPool>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT;
      if constexpr (std::is_same_v<T, VkSurfaceKHR>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
      //if constexpr (std::is_same_v<T, VkSwapChainKHR>)
      //   return VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
      if constexpr (std::is_same_v<T, VkDebugReportCallbackEXT>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT;
      if constexpr (std::is_same_v<T, VkDisplayKHR>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT;
      if constexpr (std::is_same_v<T, VkDisplayModeKHR>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT;
      if constexpr (std::is_same_v<T, VkValidationCacheEXT>)
         return VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT;
      return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
   })();
}