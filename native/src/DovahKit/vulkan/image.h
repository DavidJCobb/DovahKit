#pragma once
#include "_vulkan.h"
#include "_memory.h"
#include "_util.h"

namespace vulkanDK {
   class buffer;
   class command_buffer;
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
      VkComponentMapping    swizzle = {
         .r = VK_COMPONENT_SWIZZLE_IDENTITY,
         .g = VK_COMPONENT_SWIZZLE_IDENTITY,
         .b = VK_COMPONENT_SWIZZLE_IDENTITY,
         .a = VK_COMPONENT_SWIZZLE_IDENTITY,
      };
      VkImageTiling         tiling       = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
      VkImageUsageFlags     usage        = 0;

      static image_metadata from_dds_header(const dds::header&);
      VkImageCreateInfo create_image_info() const;
   };

   class image_and_view : no_copy {
      public:
         image_and_view() {}
         image_and_view(surface_renderer& sr) : owner(&sr) {}
         ~image_and_view();

         image_and_view(image_and_view&&) noexcept;
         image_and_view& operator=(image_and_view&&) noexcept;
         
      public:
         surface_renderer* owner  = nullptr;
         VkImage           handle = VK_NULL_HANDLE;
         VkImageView       view   = VK_NULL_HANDLE;
         struct {
            VkAccessFlags access = 0;
            VkImageLayout layout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
         } current;
         image_metadata metadata;

         inline constexpr bool empty() const noexcept { return this->handle == VK_NULL_HANDLE && this->view == VK_NULL_HANDLE; }

         void create_basic_view(VkFormat, VkImageAspectFlags);
         void destroy_view();

         void transition_layout(VkImageAspectFlags, VkImageLayout, VkAccessFlags);
         void transition_layout(command_buffer&, VkImageAspectFlags, VkImageLayout, VkAccessFlags);
         void transition_layout(command_buffer&, VkImageAspectFlags, VkImageLayout src_layout, VkImageLayout dst_layout, VkAccessFlags src_access, VkAccessFlags dst_access);

         void teardown();
   };
   class owned_image_and_view : public image_and_view {
      public:
         owned_image_and_view() {}
         owned_image_and_view(surface_renderer& sr) : image_and_view(sr) {}
         ~owned_image_and_view();

         owned_image_and_view(owned_image_and_view&&) noexcept;
         owned_image_and_view& operator=(owned_image_and_view&&) noexcept;

         image_and_view& operator=(image_and_view&&) noexcept = delete;

      public:
         VmaAllocation memory = VK_NULL_HANDLE;

         inline constexpr bool empty() const noexcept { return image_and_view::empty() && this->memory == VK_NULL_HANDLE; }

         void create_image(const image_metadata&, VkMemoryPropertyFlags);

         void copy_content_from_buffer(VkBuffer);
         void copy_content_from_buffer(command_buffer&, VkBuffer);

         void overwrite_from_staging_buffer(const buffer&, VkImageAspectFlags, VkImageLayout, VkAccessFlags);

         void teardown();
   };
}