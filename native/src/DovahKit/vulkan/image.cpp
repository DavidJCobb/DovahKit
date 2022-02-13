#include "image.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include "command_buffer.h"
#include "physical_device.h"
#include "surface_renderer.h"
//
#include "dds/header.h"

namespace {
   VkResult _create_basic_view(VkDevice device, VkImage& image, VkImageView& view, const vulkanDK::image_metadata& meta, VkImageAspectFlags aspect) {
      assert(view == VK_NULL_HANDLE);
      //
      VkImageViewType vt;
      switch (meta.dimensions) {
         case VK_IMAGE_TYPE_1D:
            vt = VK_IMAGE_VIEW_TYPE_1D;
            if (meta.layer_count > 1)
               vt = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            break;
         case VK_IMAGE_TYPE_2D:
            vt = VK_IMAGE_VIEW_TYPE_2D;
            if (meta.layer_count > 1)
               vt = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;
         case VK_IMAGE_TYPE_3D:
            vt = VK_IMAGE_VIEW_TYPE_3D;
            break;
      }
      if (meta.is_cubemap) {
         vt = VK_IMAGE_VIEW_TYPE_CUBE;
         if (meta.layer_count > 1)
            vt = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
      }
      //
      auto view_info = VkImageViewCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image      = image,
         .viewType   = vt,
         .format     = meta.format,
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
            .levelCount     = meta.mipmap_count,
            .baseArrayLayer = 0, // don't skip layers (layers would be useful for stereoscopic 3D, etc.)
            .layerCount     = meta.layer_count,
         },
      };
      return vkCreateImageView(device, &view_info, nullptr, &view);
   }

   bool _format_has_stencil_component(VkFormat format) {
      return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
   }
}

namespace vulkanDK {
   /*static*/ image_metadata image_metadata::from_dds_header(const dds::header& header) {
      image_metadata out;
      //
      bool has_ext = header.has_extended_header();
      if (has_ext) {
         switch (header.dx10_header.dimension) {
            using enum dds::header_extension::resource_dimension;
            case texture1D:
               out.dimensions = VK_IMAGE_TYPE_1D;
               break;
            case texture2D:
               out.dimensions = VK_IMAGE_TYPE_2D;
               break;
            case texture3D:
               out.dimensions = VK_IMAGE_TYPE_3D;
               break;
         }
      }
      //
      out.extent = {
         .width  = header.width,
         .height = header.height,
         .depth  = (header.flags & dds::header::flag::has_depth) ? header.depth : 1,
      };
      out.format     = header.to_vulkan_format();
      out.is_cubemap = (header.capabilities[1] & dds::header::capabilities_1::is_cubemap);
      //
      out.layer_count = 1;
      if (has_ext)
         out.layer_count = header.dx10_header.array_size;
      if (out.is_cubemap)
         out.layer_count *= 6;
      //
      out.mipmap_count = header.mipmap_count + 1;
      //
      return out;
   }
   VkImageCreateInfo image_metadata::create_image_info() const {
      uint32_t create_flags = 0;
      if (this->is_cubemap) {
         create_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
      }
      //
      auto out = VkImageCreateInfo{
         .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
         .flags         = create_flags,
         .imageType     = this->dimensions,
         .format        = this->format, // TODO: if I write a function to convert between Qt and Vulkan format enums, we can use potentially any format, though not all cards support all formats
         .extent        = this->extent,
         .mipLevels     = this->mipmap_count,
         .arrayLayers   = this->layer_count,
         .samples       = this->samples,
         .tiling        = this->tiling,
         .usage         = this->usage,
         .sharingMode   = this->sharing,
         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      };
      if (out.extent.depth == 0)
         out.extent.depth = 1;
      //
      return out;
   }

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
      auto result = _create_basic_view(this->owner->logical_device, this->image, this->view, { .format = format }, aspect);
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

   //void concrete_image::create_image(uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
   void concrete_image::create_image(const image_metadata& meta, VkMemoryPropertyFlags properties) {
      assert(this->handle == VK_NULL_HANDLE);
      assert(this->owner);
      auto device = this->owner->logical_device;
      //
      this->metadata = meta;
      {  // Validate fields. Brace initialization, etc., result in some nested structs having incorrect members if they weren't manually specified (i.e. defaults on the containing struct are skipped).
         auto& meta = this->metadata;
         if (meta.extent.depth == 0)
            meta.extent.depth = 1;
      }
      //
      auto image_info = this->metadata.create_image_info();
      auto alloc_info = VmaAllocationCreateInfo{
         .flags          = 0,
         .usage          = VMA_MEMORY_USAGE_UNKNOWN,
         .requiredFlags  = properties,
         .preferredFlags = 0,
         .memoryTypeBits = 0,
         .pool           = nullptr,
         .pUserData      = nullptr,
         .priority       = 0,
      };
      if (vmaCreateImage(this->owner->allocator, &image_info, &alloc_info, &this->handle, &this->memory, nullptr) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::concrete_image::create_image] Failed to create image.");
      }
   }
   void concrete_image::create_basic_view(VkFormat format, VkImageAspectFlags aspect) {
      assert(this->view == VK_NULL_HANDLE);
      assert(this->owner);
      auto result = _create_basic_view(this->owner->logical_device, this->handle, this->view, this->metadata, aspect);
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
               .layerCount     = this->metadata.layer_count,
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = this->metadata.extent,
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
               .levelCount     = this->metadata.mipmap_count,
               .baseArrayLayer = 0,
               .layerCount     = this->metadata.layer_count,
            },
         };
         //
         // Handle special-case aspect masks:
         //
         if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (_format_has_stencil_component(this->metadata.format))
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
            throw std::invalid_argument("[vulkanDK::concrete_image::transition_layout] Unsupported layout transition!");
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
      if (this->view != VK_NULL_HANDLE) {
         vkDestroyImageView(this->owner->logical_device, this->view, nullptr);
         this->view = VK_NULL_HANDLE;
      }
      vmaDestroyImage(this->owner->allocator, this->handle, this->memory);
      this->handle   = VK_NULL_HANDLE;
      this->memory   = VK_NULL_HANDLE;
      this->metadata = image_metadata();
   }
   #pragma endregion
}