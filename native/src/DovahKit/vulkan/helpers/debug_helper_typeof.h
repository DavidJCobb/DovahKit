#pragma once
#include "../_vulkan.h"

namespace vulkanDK {
   template<typename T> constexpr VkObjectType debug_helper_typeof = ([]() {
      if constexpr (std::is_same_v<T, VkInstance>)
         return VK_OBJECT_TYPE_INSTANCE;
      if constexpr (std::is_same_v<T, VkPhysicalDevice>)
         return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
      if constexpr (std::is_same_v<T, VkDevice>)
         return VK_OBJECT_TYPE_DEVICE;
      if constexpr (std::is_same_v<T, VkQueue>)
         return VK_OBJECT_TYPE_QUEUE;
      if constexpr (std::is_same_v<T, VkSemaphore>)
         return VK_OBJECT_TYPE_SEMAPHORE;
      if constexpr (std::is_same_v<T, VkCommandBuffer>)
         return VK_OBJECT_TYPE_COMMAND_BUFFER;
      if constexpr (std::is_same_v<T, VkFence>)
         return VK_OBJECT_TYPE_FENCE;
      if constexpr (std::is_same_v<T, VkDeviceMemory>)
         return VK_OBJECT_TYPE_DEVICE_MEMORY;
      if constexpr (std::is_same_v<T, VkBuffer>)
         return VK_OBJECT_TYPE_BUFFER;
      if constexpr (std::is_same_v<T, VkImage>)
         return VK_OBJECT_TYPE_IMAGE;
      if constexpr (std::is_same_v<T, VkEvent>)
         return VK_OBJECT_TYPE_EVENT;
      if constexpr (std::is_same_v<T, VkQueryPool>)
         return VK_OBJECT_TYPE_QUERY_POOL;
      if constexpr (std::is_same_v<T, VkBufferView>)
         return VK_OBJECT_TYPE_BUFFER_VIEW;
      if constexpr (std::is_same_v<T, VkImageView>)
         return VK_OBJECT_TYPE_IMAGE_VIEW;
      if constexpr (std::is_same_v<T, VkShaderModule>)
         return VK_OBJECT_TYPE_SHADER_MODULE;
      if constexpr (std::is_same_v<T, VkPipelineCache>)
         return VK_OBJECT_TYPE_PIPELINE_CACHE;
      if constexpr (std::is_same_v<T, VkPipelineLayout>)
         return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
      if constexpr (std::is_same_v<T, VkRenderPass>)
         return VK_OBJECT_TYPE_RENDER_PASS;
      if constexpr (std::is_same_v<T, VkPipeline>)
         return VK_OBJECT_TYPE_PIPELINE;
      if constexpr (std::is_same_v<T, VkDescriptorSetLayout>)
         return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
      if constexpr (std::is_same_v<T, VkSampler>)
         return VK_OBJECT_TYPE_SAMPLER;
      if constexpr (std::is_same_v<T, VkDescriptorPool>)
         return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
      if constexpr (std::is_same_v<T, VkDescriptorSet>)
         return VK_OBJECT_TYPE_DESCRIPTOR_SET;
      if constexpr (std::is_same_v<T, VkFramebuffer>)
         return VK_OBJECT_TYPE_FRAMEBUFFER;
      if constexpr (std::is_same_v<T, VkCommandPool>)
         return VK_OBJECT_TYPE_COMMAND_POOL;
      if constexpr (std::is_same_v<T, VkSurfaceKHR>)
         return VK_OBJECT_TYPE_SURFACE_KHR;
      //if constexpr (std::is_same_v<T, VkSwapChainKHR>)
      //   return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
      if constexpr (std::is_same_v<T, VkDebugReportCallbackEXT>)
         return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
      if constexpr (std::is_same_v<T, VkDisplayKHR>)
         return VK_OBJECT_TYPE_DISPLAY_KHR;
      if constexpr (std::is_same_v<T, VkDisplayModeKHR>)
         return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
      if constexpr (std::is_same_v<T, VkValidationCacheEXT>)
         return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
      return VK_OBJECT_TYPE_UNKNOWN;
   })();
}