#pragma once
#include "device.h"

namespace DovahKit::vulkan {
   class image {
      protected;
         device& owner;
      public:
         VkImage     handle = VK_NULL_HANDLE;
         VkImageView view   = VK_NULL_HANDLE;

         image(device&);
         ~image();
   };
}