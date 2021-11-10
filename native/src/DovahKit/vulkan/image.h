#pragma once
#include "_vulkan.h"
#include "_util.h"

namespace vulkanDK {
   class surface_renderer;

   class surface_renderer_image_view : no_copy {
      //
      // An image and view with no underlying mapped memory; good for images whose contents 
      // are not fully under your control, like swap chain images.
      //
      public:
         surface_renderer_image_view() {}
         surface_renderer_image_view(surface_renderer&, VkImage);
         ~surface_renderer_image_view();

         surface_renderer_image_view(surface_renderer_image_view&&) noexcept;
         surface_renderer_image_view& operator=(surface_renderer_image_view&&) noexcept;

         surface_renderer* owner = nullptr;
         VkImage           image = VK_NULL_HANDLE; // either use the constructor with args, or always write to this AND set an (owner) pointer
         VkImageView       view  = VK_NULL_HANDLE;

         void create_basic_view(VkFormat, VkImageAspectFlags);
         void destroy_view();
   };

   class concrete_image : no_copy {
      //
      // An image, view, and underlying mapped memory; this is an image whose contents 
      // you manage and control entirely on your own.
      //
      public:
         concrete_image() {} // if you default-construct a concrete image, you MUST replace it with an owned image (constructor with args).
         concrete_image(surface_renderer& c) : owner(&c) {}
         ~concrete_image();

         concrete_image(concrete_image&&) noexcept;
         concrete_image& operator=(concrete_image&&) noexcept;

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