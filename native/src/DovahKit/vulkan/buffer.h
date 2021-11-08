#pragma once
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class device;
   class context;

   class buffer : no_copy {
      friend class device;
      protected:
         VkDeviceMemory memory = VK_NULL_HANDLE; // temporary; replace with heap
      public:
         buffer() {}
         buffer(device&);
         ~buffer();

         buffer(buffer&&) noexcept;
         buffer& operator=(buffer&&) noexcept;
      
         device*      owner  = nullptr;
         VkBuffer     handle = VK_NULL_HANDLE;
         VkDeviceSize size   = 0;

         void copy_from(context&, const buffer& source);
         void copy_from(context&, const buffer& source, VkDeviceSize);
   };
}