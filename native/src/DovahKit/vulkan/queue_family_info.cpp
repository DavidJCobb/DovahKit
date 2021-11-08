#include "queue_family_info.h"
#include <cassert>
#include <vector>
#include "context.h"

namespace vulkanDK {
   queue_family_info::queue_family_info(context& ct) {
      auto device  = ct.physical_device();
      auto surface = ct.surface;
      //
      uint32_t count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
      std::vector<VkQueueFamilyProperties> list(count);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &count, list.data());
      //
      for (size_t i = 0; i < count; ++i) {
         const auto& family = list[i];
         //
         if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            this->set(this->families.graphics, i);
         }
         {  // Can this device render to our render window widget?
            VkBool32 support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
            if (support) {
               this->set(this->families.presentation, i);
            }
         }
         if (this->mask == all_mask_bits_set) // early out; device supports all desired queue family types
            break;
      }
   }
   void queue_family_info::set(queue_index_t& entry, queue_index_t value) {
      entry = value;
      //
      auto base = (std::intptr_t)families.list.data();
      auto item = (std::intptr_t)&entry;
      item -= base;
      item /= sizeof(queue_index_t);
      //
      this->mask |= (1 << item);
   }
   bool queue_family_info::has(const queue_index_t& entry) const noexcept {
      auto base = (std::intptr_t)families.list.data();
      auto item = (std::intptr_t)&entry;
      assert(item >= base && item < (base + families.list.size() * sizeof(queue_index_t))); // Ensure (entry) is actually an entry in our list.
      item -= base;
      item /= sizeof(queue_index_t);
      return (mask & (1 << item)) != 0;
   }
}