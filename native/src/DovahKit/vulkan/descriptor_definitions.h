#pragma once
#include <cstdint>
#include <vector>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace DovahKit::vulkan {
   class descriptor_binding {
      public:
         uint32_t                 index;
         VkDescriptorBindingFlags flags = 0;
         VkDescriptorType         type;
         uint32_t                 count;
         VkShaderStageFlags       shader_stages;
         const VkSampler*         immutable_samplers = nullptr;
         //
         bool is_global = false; // if false, then we want (count * frames) descriptors; else, (count) descriptors
      
         VkDescriptorSetLayoutBinding setup_params() const;
   };

   class descriptor_set_layout {
      protected:
         VkDevice device = VK_NULL_HANDLE;
      public:
         descriptor_set_layout() {};
         descriptor_set_layout(VkDevice);
         ~descriptor_set_layout();
      
         VkDescriptorSetLayout handle = VK_NULL_HANDLE;
         std::vector<descriptor_binding> bindings;
      
         inline VkDevice get_device() const noexcept { return this->device; }
         void set_device(VkDevice);
   
         void setup();
         void teardown();
   };
}