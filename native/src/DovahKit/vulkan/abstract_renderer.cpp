#include "abstract_renderer.h"
#include <cassert>
#include <stdexcept>
#include "exceptions.h"
#include "render_pass.h"
#include "shader_module.h"

namespace vulkanDK {
   #pragma region queue
   void abstract_renderer::queue::setup(VkDevice device, uint32_t index) {
      vkGetDeviceQueue(device, index, 0, &this->handle);
      this->index = index;
   }
   #pragma endregion

   abstract_renderer::~abstract_renderer() {
      this->start_teardown();
      this->end_teardown();
   }

   void abstract_renderer::start_teardown() {
      if (this->logical_device == VK_NULL_HANDLE)
         return;
      //
      // Subclasses should call vkDeviceWaitIdle and tear down state unique to themselves before 
      // using a call-super for this.
      //
      this->teardown_command_pool();
      this->teardown_descriptor_pool();
      this->teardown_texture_sampler();
      {
         auto& list = this->render_passes;
         for (auto* rp : list)
            if (rp)
               delete rp;
         list.clear();
      }
      {
         auto& list = this->shader_modules;
         for (auto* sm : list) {
            delete sm;
         }
         list.clear();
      }
   }
   void abstract_renderer::end_teardown() {
      if (this->logical_device == VK_NULL_HANDLE)
         return;
      vkDestroyDevice(this->logical_device, nullptr);
      this->logical_device = VK_NULL_HANDLE;
   }

   void abstract_renderer::setup_command_pool(uint32_t queue_family_index) {
      auto pool_info = VkCommandPoolCreateInfo{
         .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         .queueFamilyIndex = queue_family_index,
      };
      if (auto result = vkCreateCommandPool(this->logical_device, &pool_info, nullptr, &this->command_pools.persistent); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::abstract_renderer::_setup_command_pool] Failed to create the persistent command pool.");
      }
      //
      pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
      if (auto result = vkCreateCommandPool(this->logical_device, &pool_info, nullptr, &this->command_pools.transient); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::abstract_renderer::_setup_command_pool] Failed to create the transient command pool.");
      }
   }
   void abstract_renderer::teardown_command_pool() {
      if (this->command_pools.persistent != VK_NULL_HANDLE) {
         vkDestroyCommandPool(this->logical_device, this->command_pools.persistent, nullptr);
         this->command_pools.persistent = VK_NULL_HANDLE;
      }
      if (this->command_pools.transient != VK_NULL_HANDLE) {
         vkDestroyCommandPool(this->logical_device, this->command_pools.transient, nullptr);
         this->command_pools.transient = VK_NULL_HANDLE;
      }
   }

   void abstract_renderer::setup_descriptor_pool(const std::vector<VkDescriptorPoolSize>& sizes, uint32_t total_set_count) {
      auto pool_info = VkDescriptorPoolCreateInfo{
         .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
         .pNext         = nullptr,
         .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
         .maxSets       = total_set_count,
         .poolSizeCount = (uint32_t)sizes.size(),
         .pPoolSizes    = sizes.data(),
      };
      if (auto result = vkCreateDescriptorPool(this->logical_device, &pool_info, nullptr, &this->descriptor_pool); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::abstract_renderer::setup_descriptor_pool] Failed to create the descriptor pool.");
      }
   }
   void abstract_renderer::teardown_descriptor_pool() {
      if (this->descriptor_pool != VK_NULL_HANDLE) {
         vkDestroyDescriptorPool(this->logical_device, this->descriptor_pool, nullptr);
         this->descriptor_pool = VK_NULL_HANDLE;
      }
   }

   void abstract_renderer::setup_texture_sampler() {
      if (!this->device_info) {
         throw std::logic_error("[vulkanDK::abstract_renderer::_setup_texture_sampler] No physical device set.");
      }
      //
      const auto& support = this->device_info->support;
      //
      auto sampler_info = VkSamplerCreateInfo{
         .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
         .magFilter        = VK_FILTER_LINEAR,
         .minFilter        = VK_FILTER_LINEAR,
         .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
         .addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .mipLodBias       = 0.0,
         .anisotropyEnable = support.max_anisotropic_filtering > 0.0 ? VK_TRUE : VK_FALSE,
         .maxAnisotropy    = std::min(8.0F, support.max_anisotropic_filtering),
         .compareEnable    = VK_FALSE,
         .compareOp        = VK_COMPARE_OP_ALWAYS,
         .minLod           = 0.0,
         .maxLod           = 0.0,
         .borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
         .unnormalizedCoordinates = VK_FALSE, // true: coordinates are [0, width], etc; false: coordinates are [0, 1]
      };
      if (auto result = vkCreateSampler(this->logical_device, &sampler_info, nullptr, &this->texture_sampler); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::abstract_renderer::_setup_texture_sampler] Failed to create the texture sampler.");
      }
   }
   void abstract_renderer::teardown_texture_sampler() {
      if (this->texture_sampler != VK_NULL_HANDLE) {
         vkDestroySampler(this->logical_device, this->texture_sampler, nullptr);
         this->texture_sampler = VK_NULL_HANDLE;
      }
   }
}