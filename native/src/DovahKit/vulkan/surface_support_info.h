#pragma once
#include "_vulkan.h"

namespace vulkanDK {
   class surface_renderer;

   struct surface_support_info {
      VkSurfaceCapabilitiesKHR        capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR>   presentation_modes;
      
      surface_support_info() {}
      surface_support_info(surface_renderer&);
   };
}
