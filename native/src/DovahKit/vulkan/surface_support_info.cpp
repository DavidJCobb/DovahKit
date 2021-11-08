#include "surface_support_info.h"
#include "context.h"

namespace vulkanDK {
   surface_support_info::surface_support_info(context& c) {
      auto device  = c.owner.physical;
      auto surface = c.surface;
      //
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &this->capabilities);
      {
         uint32_t count;
         vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
         if (count != 0) {
            this->formats.resize(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, this->formats.data());
         }
      }
      {
         uint32_t count;
         vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
         if (count != 0) {
            this->presentation_modes.resize(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, this->presentation_modes.data());
         }
      }
   }
}