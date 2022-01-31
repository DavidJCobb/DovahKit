#pragma once
#include "vulkan/_vulkan.h"
#include "vulkan/_util.h"

namespace vulkanDK {
   class surface_renderer;

   class unmanaged_image : no_copy {
      //
      // An image, view, and underlying mapped memory; this is an image whose contents 
      // you manage and control entirely on your own.
      //
      public:
         unmanaged_image() {} // if you default-construct a concrete image, you MUST replace it with an owned image (constructor with args).
         unmanaged_image(surface_renderer& c) : owner(&c) {}
         ~unmanaged_image();

         unmanaged_image(unmanaged_image&&) noexcept;
         unmanaged_image& operator=(unmanaged_image&&) noexcept;

         surface_renderer* owner = nullptr;
         VkImage        handle = VK_NULL_HANDLE;
         VkImageView    view   = VK_NULL_HANDLE;
         VkDeviceMemory memory = VK_NULL_HANDLE;
         //
         VkFormat format = VK_FORMAT_UNDEFINED;
         struct {
            uint32_t w = 0;
            uint32_t h = 0;
         } size;

         void create_image(uint32_t w, uint32_t h, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags);
         void create_basic_view(VkFormat, VkImageAspectFlags);

         void copy_content_from_buffer(VkBuffer);

         void transition_layout(VkImageLayout old_layout, VkImageLayout new_layout);

         void teardown();
   };
}