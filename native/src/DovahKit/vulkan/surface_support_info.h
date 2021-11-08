#pragma once
#include "_vulkan.h"
#include "context.h"

namespace vulkanDK {
   class context;

   struct surface_support_info {
      VkSurfaceCapabilitiesKHR        capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR>   presentation_modes;
      
      surface_support_info() {}
      surface_support_info(context&);
   };
}
