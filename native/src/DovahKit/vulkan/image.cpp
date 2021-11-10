#include "image.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include "command_buffer.h"
#include "physical_device.h"
#include "surface_renderer.h"

namespace {
   VkResult _create_basic_view(VkDevice device, VkImage& image, VkImageView& view, VkFormat format, VkImageAspectFlags aspect) {
      assert(view == VK_NULL_HANDLE);
      //
      auto view_info = VkImageViewCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image      = image,
         .viewType   = VK_IMAGE_VIEW_TYPE_2D,
         .format     = format,
         .components = {
            //
            // No color channel mixing/swapping/etc.
            //
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
         },
         .subresourceRange = { // control what part of the image is accessed
            .aspectMask     = aspect,
            .baseMipLevel   = 0, // don't skip mipmaps
            .levelCount     = 1, // don't use mipmaps
            .baseArrayLayer = 0, // don't skip layers (layers would be useful for stereoscopic 3D, etc.)
            .layerCount     = 1, // only one layer
         },
      };
      return vkCreateImageView(device, &view_info, nullptr, &view);
   }

   bool _format_has_stencil_component(VkFormat format) {
      return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
   }
}

namespace vulkanDK {
   #pragma region surface_renderer_image_view
   surface_renderer_image_view::surface_renderer_image_view(surface_renderer& o, VkImage i) : owner(&o), image(i) {}
   surface_renderer_image_view::~surface_renderer_image_view() {
      this->destroy_view();
   }


   surface_renderer_image_view::surface_renderer_image_view(surface_renderer_image_view&& o) noexcept {
      std::swap(this->owner, o.owner);
      std::swap(this->image, o.image);
      std::swap(this->view,  o.view);
   }
   surface_renderer_image_view& surface_renderer_image_view::operator=(surface_renderer_image_view&& o) noexcept {
      std::swap(this->owner, o.owner);
      std::swap(this->image, o.image);
      std::swap(this->view,  o.view);
      return *this;
   }
   
   void surface_renderer_image_view::create_basic_view(VkFormat format, VkImageAspectFlags aspect) {
      assert(this->view == VK_NULL_HANDLE);
      assert(this->owner);
      auto result = _create_basic_view(this->owner->logical_device, this->image, this->view, format, aspect);
      if (result != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer_image_view::create_basic_view] Failed to create texture image view.");
      }
   }
   void surface_renderer_image_view::destroy_view() {
      if (this->view == VK_NULL_HANDLE)
         return;
      assert(this->owner);
      vkDestroyImageView(this->owner->logical_device, this->view, nullptr);
      this->view = VK_NULL_HANDLE;
   }
   #pragma endregion

   #pragma region concrete_image
   concrete_image::~concrete_image() {
      this->teardown();
   }


   concrete_image::concrete_image(concrete_image&& o) noexcept {
      std::swap(this->owner,  o.owner);
      std::swap(this->handle, o.handle);
      std::swap(this->view,   o.view);
      std::swap(this->memory, o.memory);
   }
   concrete_image& concrete_image::operator=(concrete_image&& o) noexcept {
      std::swap(this->owner,  o.owner);
      std::swap(this->handle, o.handle);
      std::swap(this->view,   o.view);
      std::swap(this->memory, o.memory);
      return *this;
   }

   void concrete_image::create_image(uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
      assert(this->handle == VK_NULL_HANDLE);
      assert(this->owner);
      auto device = this->owner->logical_device;
      //
      auto image_info = VkImageCreateInfo{
         .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
         .flags     = 0,
         .imageType = VK_IMAGE_TYPE_2D,
         .format    = format, // TODO: if I write a function to convert between Qt and Vulkan format enums, we can use potentially any format, though not all cards support all formats
         .extent    = {
            .width  = w,
            .height = h,
            .depth  = 1,
         },
         .mipLevels     = 1,
         .arrayLayers   = 1,
         .samples       = VK_SAMPLE_COUNT_1_BIT,
         .tiling        = VK_IMAGE_TILING_OPTIMAL,
         .usage         = usage,
         .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      };
      if (vkCreateImage(device, &image_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("failed to create image!");
      }
      this->format = format;
      this->size.w = w;
      this->size.h = h;
      //
      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements(device, this->handle, &memRequirements);
      //
      auto alloc_info = VkMemoryAllocateInfo{
         .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize  = memRequirements.size,
         .memoryTypeIndex = this->owner->device_info->find_memory_type(memRequirements.memoryTypeBits, properties),
      };
      if (vkAllocateMemory(device, &alloc_info, nullptr, &this->memory) != VK_SUCCESS) {
         throw std::runtime_error("failed to allocate image memory!");
      }
      //
      vkBindImageMemory(device, this->handle, this->memory, 0);
   }
   void concrete_image::create_basic_view(VkFormat format, VkImageAspectFlags aspect) {
      assert(this->view == VK_NULL_HANDLE);
      assert(this->owner);
      auto result = _create_basic_view(this->owner->logical_device, this->handle, this->view, format, aspect);
      if (result != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::concrete_image::create_basic_view] Failed to create texture image view.");
      }
   }

   void concrete_image::copy_content_from_buffer(VkBuffer buffer) {
      assert(this->owner);
      this->owner->do_single_commands([this, buffer](command_buffer& scratch_commands) {
         auto region = VkBufferImageCopy{
            .bufferOffset      = 0,
            .bufferRowLength   = 0, // amount of padding bytes between rows?
            .bufferImageHeight = 0, // amount of padding bytes... somewhere?
            .imageSubresource  = {
               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
               .mipLevel       = 0,
               .baseArrayLayer = 0,
               .layerCount     = 1,
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { this->size.w, this->size.h, 1 },
         };
         vkCmdCopyBufferToImage(
            scratch_commands.handle,
            buffer,
            this->handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
         );
      });
   }

   void concrete_image::transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) {
      assert(this->owner);
      assert(this->handle != VK_NULL_HANDLE);
      this->owner->do_single_commands([this, old_layout, new_layout](command_buffer& scratch_commands) {
         auto barrier = VkImageMemoryBarrier{
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask       = 0,
            .dstAccessMask       = 0,
            .oldLayout           = old_layout, // can use VK_IMAGE_LAYOUT_UNDEFINED if you don't care to preserve the image's existing content
            .newLayout           = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = this->handle,
            .subresourceRange    = {
               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
               .baseMipLevel   = 0,
               .levelCount     = 1,
               .baseArrayLayer = 0,
               .layerCount     = 1,
            },
         };
         //
         // Handle special-case aspect masks:
         //
         if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (_format_has_stencil_component(this->format))
               barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
         }
         //
         // We need to set up the proper access masks and indicate when (i.e. during what pipeline 
         // stages) we can read and write. We need to handle different transitions here, so we'll 
         // need to extend this function as we add more.
         //
         VkPipelineStageFlags sourceStage;
         VkPipelineStageFlags destinationStage;
         if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            //
            // If we're going from an undefined layout (i.e. we don't care about the image's prior 
            // content) to a transfer-destination layout, then our transfer writes don't need to 
            // wait on anything.
            // 
            // Because transfer writes don't need to wait, we can specify an empty access mask and 
            // use the earliest possible pipeline stage: "top of pipe."
            //
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            //
            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
         } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            //
            // If we're going from a transfer-destination layout to a shader-read-only layout (i.e. 
            // the fragment shader wants to read the image), then we need to wait on transfer writes.
            //
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // wait on transfer writes
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;    // we're doing a shader read
            //
            sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // do this when processing the fragment shader
         } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            //
            // Transition used when creating a new depth image for our depth buffer.
            //
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            //
            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
         } else {
            throw std::invalid_argument("[DovahKitVulkanSubsystem][transitionImageLayout] Unsupported layout transition!");
         }

         vkCmdPipelineBarrier(
            scratch_commands.handle,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
         );
      });
   }

   void concrete_image::teardown() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      assert(this->owner);
      auto device = this->owner->logical_device;
      vkDestroyImageView(device, this->view,   nullptr);
      vkDestroyImage    (device, this->handle, nullptr);
      vkFreeMemory      (device, this->memory, nullptr);
      this->view   = VK_NULL_HANDLE;
      this->handle = VK_NULL_HANDLE;
      this->memory = VK_NULL_HANDLE;
      this->format = VK_FORMAT_UNDEFINED;
      this->size.w = 0;
      this->size.h = 0;
   }
   #pragma endregion
}