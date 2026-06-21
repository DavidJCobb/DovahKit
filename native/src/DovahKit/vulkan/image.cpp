#include "image.h"
#include <algorithm>
#include <cassert>
#include "./buffer.h"
#include "./command_buffer.h"
#include "./exceptions.h"
#include "./physical_device.h"
#include "./surface_renderer.h"
//
#include "./data/vulkan_formats.h"
#include "./dds/header.h"
#include "./helpers/calc_texture_bytecount.h"
#include "./helpers/convert_access_flags_and_pipeline_stages.h"

namespace {
   VkResult _create_basic_view(VkDevice device, VkImage& image, VkImageView& view, const vulkanDK::image_metadata& meta, VkImageAspectFlags aspect) {
      assert(view == VK_NULL_HANDLE);
      //
      VkImageViewType vt = VK_IMAGE_VIEW_TYPE_2D;
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
         .components = meta.swizzle,
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
   #pragma region image_metadata
   /*static*/ image_metadata image_metadata::from_dds_header(const dds::header& header, size_t pixel_data_size) {
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
      out.is_cubemap = (header.capabilities[1] & dds::header::capabilities_1::is_cubemap);
      out.format     = header.to_vulkan_format();
      if (out.format == VK_FORMAT_UNDEFINED) {
         //
         // DDS files can define color components in varying orders, but Vulkan basically 
         // only has formats defined for RGBA. However, by swizzling the image components, 
         // we can handle other orders.
         //
         const auto& pf = header.format;
         if (pf.has_rgb_bitcount() && !pf.channels_overlap()) {
            const auto bitcount = pf.uniform_channel_bitcount();
            if (bitcount) {
               //
               // The image has color channels that don't overlap, and all channels use 
               // the same bitcount. We should be able to handle this through swizzling.
               //
               auto    sequence = pf.channel_sequence();
               uint8_t count    = 0;
               for (uint8_t i = 0; i < 4; ++i) {
                  if (sequence[i].first != -1)
                     ++count;
                  else
                     break;
               }
               //
               VkFormat potential_format = VkFormat::VK_FORMAT_UNDEFINED;
               switch (count) {
                  case 1:
                     switch (bitcount) {
                        case  8: potential_format = VkFormat::VK_FORMAT_R8_UNORM; break;
                        case 16: potential_format = VkFormat::VK_FORMAT_R16_UNORM; break;
                        case 32: potential_format = VkFormat::VK_FORMAT_R32_UINT; break; // no UNORM variant. maybe not usable? UINT maps to [0, 255]... as in, the shader sees [0, 255] when it expects [0, 1].
                     }
                     break;
                  case 2:
                     switch (bitcount) {
                        case  8: potential_format = VkFormat::VK_FORMAT_R8G8_UNORM; break;
                        case 16: potential_format = VkFormat::VK_FORMAT_R16G16_UNORM; break;
                     }
                     break;
                  case 3:
                     switch (bitcount) {
                        case 8: potential_format = VkFormat::VK_FORMAT_R8G8B8_UNORM; break;
                     }
                     break;
                  case 4:
                     switch (bitcount) {
                        case 8: potential_format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM; break;
                     }
                     break;
               }
               if (potential_format != VkFormat::VK_FORMAT_UNDEFINED) {
                  auto _set_swizzle = [](int8_t component, const int8_t key, VkComponentSwizzle& out) -> void {
                     /*//
                     if (key == component) {
                        out = VK_COMPONENT_SWIZZLE_IDENTITY;
                        return;
                     }
                     //*/
                     switch (key) {
                        case  0: out = VK_COMPONENT_SWIZZLE_R; break;
                        case  1: out = VK_COMPONENT_SWIZZLE_G; break;
                        case  2: out = VK_COMPONENT_SWIZZLE_B; break;
                        case  3: out = VK_COMPONENT_SWIZZLE_A; break;
                        case -1:
                           out = (component == 3) ? VK_COMPONENT_SWIZZLE_ONE : VK_COMPONENT_SWIZZLE_ZERO;
                           break;
                     }
                  };
                  //
                  _set_swizzle(0, sequence[0].first, out.swizzle.r);
                  _set_swizzle(1, sequence[1].first, out.swizzle.g);
                  _set_swizzle(2, sequence[2].first, out.swizzle.b);
                  _set_swizzle(3, sequence[3].first, out.swizzle.a);
               }
               out.format = potential_format;
            }
         }
      }
      //
      out.layer_count = 1;
      if (has_ext)
         out.layer_count = header.dx10_header.array_size;
      if (out.is_cubemap)
         out.layer_count *= 6;
      //
      out.mipmap_count = header.mipmap_count;
      out.mipmap_count = std::min(
         out.mipmap_count,
         // maximum possible number of mip levels, based on image dimensions:
         (decltype(out.mipmap_count)) std::bit_width(std::min(header.width, header.height))
      );
      {
         //
         // Mipmaps are messy for two reasons. First: a DDS file may specify the "has 
         // mipmaps" flags but leave the mipmap count at zero, to imply that the file 
         // contains mipmaps all the way down to 1x1. Second: to transfer mipmaps to 
         // the GPU via Vulkan, we have to know the file offsets of each mipmap level. 
         // Those can be computed easily for all known texture formats.
         //
         auto size_query = helpers::get_texture_bytecount_calc_function(out.format);
         if (size_query == nullptr) {
            //
            // We won't be able to figure out the mipmap levels' file offsets, so we 
            // have to use only the top-level texture.
            //
            out.mipmap_count = 1;
         } else {
            if (header.mipmap_count == 0 && pixel_data_size != 0) {
               //
               // Our mipmap count is zero. We have the total size of all image data, 
               // across all mipmap levels, so let's see if that size suggests that 
               // we have mipmap levels.
               // 
               // Adding mipmaps down to 1x1px will increase a file's size by 33%. We 
               // can get the size of the top-level image, and see if our file size 
               // is roughly 33% larger than that (with a little leeway for safety).
               //
               size_t expected_top_level_size = 0;
               if ((header.flags & dds::header::flag::has_linear_size) && header.linear_size != 0) {
                  expected_top_level_size = header.linear_size;
               } else {
                  expected_top_level_size = size_query(out.extent.width, out.extent.height);
               }
               expected_top_level_size *= out.layer_count;
               //
               if (pixel_data_size >= expected_top_level_size * 1.30) {
                  //
                  // Mipmaps probably extend down to 1x1.
                  //
                  out.mipmap_count = std::bit_width(std::min(header.width, header.height));
               } else {
                  //
                  // Ensure at least one usable layer (i.e. the main image).
                  //
                  out.mipmap_count = 1;
               }
               //
               // NOTE: DirectX 9 just treats a count of 0 as if it were 1, rather 
               //       than trying to deduce the number of mip levels.
               // See: https://github.com/microsoft/DirectXTex/issues/43#issuecomment-268435766
            }
         }
      }
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

   void image_metadata::get_mip_level_offsets(std::vector<size_t>& out) {
      out.clear();
      if (this->mipmap_count == 0) {
         out.resize(1);
         out[0] = 0;
         return;
      }
      out.resize(this->mipmap_count);
      out[0] = 0;

      auto calc = helpers::get_texture_bytecount_calc_function(this->format);
      if (!calc) {
         out.resize(1);
         return;
      }
      auto w = this->extent.width;
      auto h = this->extent.height;
      for (size_t i = 1; i < this->mipmap_count; ++i) {
         out[i] = out[i - 1];
         out[i] += calc(w, h) * this->layer_count;
         //
         // Divide these afterward: we want the offset of each level, not the 
         // size. The offset of a mip level is the cumulative size of all the 
         // previous levels.
         //
         w /= 2;
         h /= 2;
      }
   }
   #pragma endregion

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
      this->owner->do_single_commands(
         [=](command_buffer& scratch_commands) {
            this->transition_layout(scratch_commands, aspect, layout, access);
         },
         //
         // The pipeline barriers used for some image layout transitions are queue-specific, so 
         // force the graphics queue for this.
         //
         this->owner->queues.graphics
      );
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
      this->owner->do_single_commands(
         [this, &staging, aspect, layout, access](command_buffer& scratch_commands) {
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
         },
         //
         // Graphics queue, not transfer queue; see comments on transition_layout
         //
         this->owner->queues.graphics
      );
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