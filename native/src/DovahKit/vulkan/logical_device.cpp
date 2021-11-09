#include "logical_device.h"
#include <stdexcept>
#include "queue_family_info.h"
#include "surface_renderer.h"
#include "config/validation_layers.h"

namespace {
   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
   };
}

namespace vulkanDK {
   logical_device::logical_device(DKVulkanInstance& o, const physical_device& pd, const surface& target_surface) : owner(o), physical(pd) {
      auto  indices        = queue_family_info(pd, target_surface);
      float queue_priority = 1.0F;
      std::vector<VkDeviceQueueCreateInfo> queue_infos;
      {
         queue_infos.reserve(queue_family_info::unique_family_count);
         //
         for (size_t i = 0; i < queue_family_info::unique_family_count; ++i) {
            if (!indices.has_index(i))
               continue;
            bool already_used = false;
            for (size_t j = 0; j < i; ++j) {
               if (indices.families.list[j] == indices.families.list[i]) {
                  already_used = true;
                  break;
               }
            }
            if (already_used)
               //
               // It's possible for queue families to share an index, but we need to make 
               // sure that we create only one queue-info for each index.
               //
               continue;
            //
            queue_infos.push_back(VkDeviceQueueCreateInfo{
               .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
               .queueFamilyIndex = indices.families.list[i],
               .queueCount       = 1,
               .pQueuePriorities = &queue_priority,
            });
         }
      }
      //
      auto deviceFeatures = VkPhysicalDeviceFeatures{
         .samplerAnisotropy = pd.support.max_anisotropic_filtering > 0 ? VK_TRUE : VK_FALSE,
      };
      //
      auto robustness_extensions = VkPhysicalDeviceRobustness2FeaturesEXT{
         .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
         .pNext               = nullptr,
         .robustBufferAccess2 = VK_FALSE,
         .robustImageAccess2  = VK_FALSE,
         .nullDescriptor      = pd.support.descriptor_bindings.null_handles ? VK_TRUE : VK_FALSE,
      };
      auto indexing_extensions = VkPhysicalDeviceDescriptorIndexingFeaturesEXT{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
         .pNext = &robustness_extensions,
         .descriptorBindingPartiallyBound          = VK_TRUE,
         .descriptorBindingVariableDescriptorCount = VK_TRUE,
         .runtimeDescriptorArray                   = VK_TRUE,
      };
      auto create_info = VkDeviceCreateInfo{
         .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
         .pNext                   = &indexing_extensions,
         .queueCreateInfoCount    = (uint32_t)queue_infos.size(),
         .pQueueCreateInfos       = queue_infos.data(),
         .enabledExtensionCount   = (uint32_t)device_extensions.size(),
         .ppEnabledExtensionNames = device_extensions.data(),
         .pEnabledFeatures        = &deviceFeatures,
      };
      if (config::enable_validation_layers) {
         create_info.enabledLayerCount   = static_cast<uint32_t>(config::desired_validation_layers.size());
         create_info.ppEnabledLayerNames = config::desired_validation_layers.data();
      } else {
         create_info.enabledLayerCount = 0;
      }
      //
      if (vkCreateDevice(pd.handle, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         // report VK_ERROR_DEVICE_LOST
         throw std::runtime_error("[vulkanDK::logical_device] Failed to create logical device.");
      }
      //
      // And lastly, let's get our queues:
      //
      vkGetDeviceQueue(this->handle, indices.families.graphics,     0, &this->queues.graphics);
      vkGetDeviceQueue(this->handle, indices.families.presentation, 0, &this->queues.presentation);
   }
   logical_device::~logical_device() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      vkDeviceWaitIdle(this->handle); // wait until device is no longer in use
      //
      // Tear down dependent objects:
      //
      {
         auto& list = this->dependent_objects.surface_renderers;
         for (auto* obj : list)
            obj->teardown();
         list.clear();
      }
      //
      vkDestroyDevice(this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
   }

   logical_device::logical_device(logical_device&& o) noexcept : owner(o.owner), physical(o.physical) {
      std::swap(this->queues, o.queues);
      std::swap(this->handle, o.handle);
      //
      std::swap(this->dependent_objects, o.dependent_objects);
   }

   
   buffer logical_device::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
      buffer out = buffer(*this);
      //
      auto buffer_info = VkBufferCreateInfo{
         .sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size         = size,
         .usage        = usage,
         .sharingMode  = VK_SHARING_MODE_EXCLUSIVE,
      };
      if (vkCreateBuffer(this->handle, &buffer_info, nullptr, &out.handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::device::create_buffer] Failed to create vertex buffer.");
      }
      //
      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(this->handle, out.handle, &memRequirements);
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
         .memoryTypeIndex = this->physical.find_memory_type(memRequirements.memoryTypeBits, properties),
      };
      if (vkAllocateMemory(this->handle, &alloc_info, nullptr, &out.memory) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::logical_device::create_buffer] Failed to allocate vertex buffer memory.");
      }

      vkBindBufferMemory(this->handle, out.handle, out.memory, 0);
   }


   void logical_device::on_dependent_object_created(surface_renderer& o) {
      this->dependent_objects.surface_renderers.push_back(&o);
   }
   void logical_device::on_dependent_object_deleted(surface_renderer& o) {
      auto& list = this->dependent_objects.surface_renderers;
      auto  it   = std::find(list.begin(), list.end(), &o);
      if (it != list.end())
         list.erase(it);
   }

}