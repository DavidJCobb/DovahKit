#pragma once
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class device;
   class surface_renderer;

   class image : no_copy {
      public:
         image() {}
         image(device&);
         ~image();

         image(image&&) noexcept;
         image& operator=(image&&) noexcept;

         device* owner  = nullptr;
         VkImage handle = VK_NULL_HANDLE;
   };

   class image_and_view : no_copy {
      public:
         ~image_and_view();
      
         image       content;
         VkImageView view = VK_NULL_HANDLE;

         void create_basic_view(VkFormat, VkImageAspectFlags);
         void destroy_view();
   };

   class concrete_image : public image_and_view {
      public:
         concrete_image(); // if you default-construct a concrete image, you MUST replace it with an owned image (constructor with args).
         concrete_image(surface_renderer& c) : owner(&c) {}
         ~concrete_image();

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