#pragma once
#include "_vulkan.h"
#include "_memory.h"
#include "_util.h"

namespace vulkanDK {
   class surface_renderer;
   namespace dds {
      struct header;
   }

   struct image_metadata {
      VkImageType           dimensions   = VkImageType::VK_IMAGE_TYPE_2D;
      VkExtent3D            extent       = { .width = 0, .height = 0, .depth = 1 };
      VkFormat              format       = VkFormat::VK_FORMAT_UNDEFINED;
      bool                  is_cubemap   = false;
      uint32_t              layer_count  = 1; // includes top-level texture, unlike DDS; for cubemaps, this is the total face count (cube count * 6)
      uint32_t              mipmap_count = 1; // includes top-level texture, unlike DDS
      VkSampleCountFlagBits samples      = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
      VkSharingMode         sharing      = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
      VkImageTiling         tiling       = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
      VkImageUsageFlags     usage        = 0;

      static image_metadata from_dds_header(const dds::header&);
      VkImageCreateInfo create_image_info() const;
   };

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

         surface_renderer* owner  = nullptr;
         VkImage           handle = VK_NULL_HANDLE;
         VkImageView       view   = VK_NULL_HANDLE;
         VmaAllocation     memory = VK_NULL_HANDLE;
         //
         image_metadata metadata;

         void create_image(const image_metadata&, VkMemoryPropertyFlags);
         void create_basic_view(VkFormat, VkImageAspectFlags);

         void copy_content_from_buffer(VkBuffer);

         void transition_layout(VkImageLayout old_layout, VkImageLayout new_layout);

         void teardown();
   };
}