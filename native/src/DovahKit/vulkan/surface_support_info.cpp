#include "surface_support_info.h"
#include "physical_device.h"
#include "surface_renderer.h"

namespace vulkanDK {
   surface_support_info::surface_support_info(surface_renderer& c) {
      assert(c.device_info);
      auto device  = c.device_info->handle;
      auto surface = c.handle;
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