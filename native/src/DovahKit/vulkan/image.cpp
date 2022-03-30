#include "image.h"
#include <algorithm>
#include <cassert>
#include "buffer.h"
#include "command_buffer.h"
#include "exceptions.h"
#include "physical_device.h"
#include "surface_renderer.h"
//
#include "dds/header.h"
#include "helpers/convert_access_flags_and_pipeline_stages.h"

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
         if (meta.layer_count > 6)
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

   #pragma region image_and_view
   image_and_view::~image_and_view() {
      this->teardown();
   }

   image_and_view::image_and_view(image_and_view&& other) noexcept {
      std::swap(this->owner,  other.owner);
      std::swap(this->handle, other.handle);
      std::swap(this->view,   other.view);
      //
      std::swap(this->metadata, other.metadata);
      std::swap(this->current,  other.current);
   }
   image_and_view& image_and_view::operator=(image_and_view&& other) noexcept {
      std::swap(this->owner,  other.owner);
      std::swap(this->handle, other.handle);
      std::swap(this->view,   other.view);
      //
      std::swap(this->metadata, other.metadata);
      std::swap(this->current,  other.current);
      //
      return *this;
   }

   void image_and_view::create_basic_view(VkFormat format, VkImageAspectFlags aspect) {
      assert(this->view == VK_NULL_HANDLE);
      assert(this->owner);
      auto meta   = this->metadata;
      meta.format = format;
      //
      auto result = _create_basic_view(this->owner->logical_device, this->handle, this->view, meta, aspect);
      if (result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::image_and_view::create_basic_view] Failed to create texture image view.");
      }
   }
   void image_and_view::destroy_view() {
      if (this->view == VK_NULL_HANDLE)
         return;
      assert(this->owner);
      vkDestroyImageView(this->owner->logical_device, this->view, nullptr);
      this->view = VK_NULL_HANDLE;
   }

   void image_and_view::transition_layout(VkImageAspectFlags aspect, VkImageLayout layout, VkAccessFlags access) {
      this->owner->do_single_commands([=](command_buffer& scratch_commands) {
         this->transition_layout(scratch_commands, aspect, layout, access);
      });
   }
   void image_and_view::transition_layout(command_buffer& cmd, VkImageAspectFlags aspect, VkImageLayout layout, VkAccessFlags access) {
      this->transition_layout(cmd, aspect, this->current.layout, layout, this->current.access, access);
   }
   void image_and_view::transition_layout(command_buffer& cmd, VkImageAspectFlags aspect, VkImageLayout src_layout, VkImageLayout dst_layout, VkAccessFlags src_access, VkAccessFlags dst_access) {
      auto barrier = VkImageMemoryBarrier{
         .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         .srcAccessMask       = src_access,
         .dstAccessMask       = dst_access,
         .oldLayout           = src_layout,
         .newLayout           = dst_layout,
         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
         .image               = this->handle,
         .subresourceRange    = {
            .aspectMask     = aspect,
            .baseMipLevel   = 0,
            .levelCount     = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount     = VK_REMAINING_ARRAY_LAYERS,
         },
      };
      //
      auto src_stage = access_flags_to_pipeline_stages(src_access, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
      auto dst_stage = access_flags_to_pipeline_stages(dst_access, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
      vkCmdPipelineBarrier(
         cmd.handle,
         src_stage, dst_stage,
         0,
         0, nullptr,
         0, nullptr,
         1, &barrier
      );
      this->current.access = dst_access;
      this->current.layout = dst_layout;
   }

   void image_and_view::teardown() {
      this->destroy_view();
      this->handle = VK_NULL_HANDLE;
   }
   #pragma endregion
   #pragma region owned_image_and_view
   owned_image_and_view::~owned_image_and_view() {
      this->teardown();
   }

   owned_image_and_view::owned_image_and_view(owned_image_and_view&& other) noexcept {
      std::swap(this->owner,  other.owner);
      std::swap(this->handle, other.handle);
      std::swap(this->view,   other.view);
      std::swap(this->memory, other.memory);
      //
      std::swap(this->metadata, other.metadata);
      std::swap(this->current,  other.current);
   }
   owned_image_and_view& owned_image_and_view::operator=(owned_image_and_view&& other) noexcept {
      std::swap(this->owner,  other.owner);
      std::swap(this->handle, other.handle);
      std::swap(this->view,   other.view);
      std::swap(this->memory, other.memory);
      //
      std::swap(this->metadata, other.metadata);
      std::swap(this->current,  other.current);
      //
      return *this;
   }
   
   void owned_image_and_view::create_image(const image_metadata& meta, VkMemoryPropertyFlags properties) {
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
      if (auto result = vmaCreateImage(this->owner->allocator, &image_info, &alloc_info, &this->handle, &this->memory, nullptr); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::owned_image_and_view::create_image] Failed to create image.");
      }
   }
   
   void owned_image_and_view::copy_content_from_buffer(VkBuffer buffer) {
      assert(this->owner);
      this->owner->do_single_commands([this, buffer](command_buffer& scratch_commands) {
         this->copy_content_from_buffer(scratch_commands, buffer);
      });
   }
   void owned_image_and_view::copy_content_from_buffer(command_buffer& cmd, VkBuffer buffer) {
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
         cmd.handle,
         buffer,
         this->handle,
         this->current.layout,
         1,
         &region
      );
   }

   void owned_image_and_view::overwrite_from_staging_buffer(const buffer& staging, VkImageAspectFlags aspect, VkImageLayout layout, VkAccessFlags access) {
      assert(this->owner);
      this->owner->do_single_commands([this, &staging, aspect, layout, access](command_buffer& scratch_commands) {
         {
            //
            // We don't care what content was present before, so we'll treat the image layout as 
            // undefined and switch to a transfer-destination layout for our copy operation.
            // 
            // Because transfer writes don't need to wait, the source access mask can be empty; 
            // based on that, we'll use the earliest pipeline stage: "top of pipe."
            //
            this->current.access = 0;
            this->current.layout = VK_IMAGE_LAYOUT_UNDEFINED;
            this->transition_layout(aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);
         }
         this->copy_content_from_buffer(scratch_commands, staging.handle);
         this->transition_layout(scratch_commands, aspect, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, access);
      });
   }

   void owned_image_and_view::teardown() {
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