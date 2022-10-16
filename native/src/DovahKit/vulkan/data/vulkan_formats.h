#pragma once
#include <array>
#include <cstdint>
#include "helpers/arrays/quicksort_ct.h"
#include "helpers/unreachable.h"
#include "../_vulkan.h"

namespace vulkanDK::data {
   enum class vulkan_format_compression_type : uint8_t {
      unspecified,
      none,
      block,             // BC
      ericcson,          // ETC2
      ericcson_alpha,    // EAC
      adaptive_scalable, // ASTC

      bc   = block,
      etc2 = ericcson,
      eac  = ericcson_alpha,
      astc = adaptive_scalable,
   };

   struct vulkan_format_info {
      VkFormat format;
      //
      struct {
         struct {
            struct {
               uint8_t w = 0;
               uint8_t h = 0;
               uint8_t d = 0; // depth
            } block_size;
         } astc;
      } compression_info;
      vulkan_format_compression_type compression_type = vulkan_format_compression_type::unspecified;
      uint8_t texel_block_size; // bytes per pixel; acts as required byte alignment within a buffer, when copying from a buffer to an image

      constexpr VkDeviceSize size_of_image(size_t w, size_t h) {
         constexpr auto _div_ceil = [](size_t a, int b) {
            return std::max(size_t(1), (a + (b - 1)) / b);
         };

         switch (compression_type) {
            using enum vulkan_format_compression_type;
            case none:
               return w * h * texel_block_size;
            case bc:
               return _div_ceil(w, 4) * _div_ceil(h, 4) * texel_block_size;
            case astc:
               {
                  auto blocks_w = _div_ceil(w, compression_info.astc.block_size.w);
                  auto blocks_h = _div_ceil(h, compression_info.astc.block_size.h);
                  auto blocks_d = 1;
                  if (compression_info.astc.block_size.d) {
                     blocks_d = (1 + (compression_info.astc.block_size.d - 1)) / compression_info.astc.block_size.d;
                  }
                  return blocks_w * blocks_h * blocks_d * texel_block_size;
               }
               break;
            case etc2:
            case eac:
               return _div_ceil(w, 4) * _div_ceil(h, 4) * texel_block_size;
         }
         cobb::unreachable();
      }
   };

   inline constexpr const auto all_vulkan_formats = []() -> std::array<vulkan_format_info, 218> {
      auto raw_list = std::array{
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R4G4_UNORM_PACK8,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8_SRGB,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R10X6_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R12X4_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R4G4B4A4_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B4G4R4A4_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R5G6B5_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B5G6R5_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R5G5B5A1_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B5G5R5A1_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8_SRGB,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8_SRGB,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8_SRGB,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8A8_SRGB,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_UNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_SNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_USCALED_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_UINT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_SINT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A8B8G8R8_SRGB_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2R10G10B10_UNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2R10G10B10_SNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2R10G10B10_USCALED_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2R10G10B10_UINT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2R10G10B10_SINT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2B10G10R10_SNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2B10G10R10_USCALED_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2B10G10R10_UINT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_A2B10G10R10_SINT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_SNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_USCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_SSCALED,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32B32_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 12,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32B32_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 12,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 12,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32B32A32_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32B32A32_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64B64_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 24,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64B64_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 24,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64B64_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 24,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64B64A64_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 32,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64B64A64_SINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 32,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 32,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_D16_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 2,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_D32_SFLOAT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_S8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 1,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_D16_UNORM_S8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_D24_UNORM_S8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 5,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC1_RGB_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC1_RGB_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC2_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC2_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC3_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC3_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC4_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC4_SNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC5_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC5_SNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC7_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_BC7_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::bc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::etc2,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::etc2,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::etc2,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::etc2,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::etc2,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
            .compression_type = vulkan_format_compression_type::etc2,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_EAC_R11_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::eac,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_EAC_R11_SNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::eac,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::eac,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
            .compression_type = vulkan_format_compression_type::eac,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 4, 4, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 4, 4, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 5, 4, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 5, 4, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 5, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 5, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 6, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 6, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 6, 6, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 6, 6, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 8, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 8, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 8, 6, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 8, 6, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 8, 8, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 8, 8, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 5, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 6, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 6, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 8, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 8, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 10, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 10, 10, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 12, 10, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 12, 10, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 12, 12, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
            .compression_info = {
               .astc = {
                  .block_size = { 12, 12, 1 },
               },
            },
            .compression_type = vulkan_format_compression_type::astc,
            .texel_block_size = 16,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G8B8G8R8_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B8G8R8G8_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 4,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 3,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G16B16G16R16_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_B16G16R16G16_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 8,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
         vulkan_format_info{
            .format           = VkFormat::VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
            .compression_type = vulkan_format_compression_type::none,
            .texel_block_size = 6,
         },
      };
      // as of VS 17.3.5, std::sort with a custom comparator causes IntelliSense to falsely complain about access one past the end of the array
      // which in turn breaks IntelliSense for everything that uses the array
      cobb::arrays::quicksort_ct(raw_list, [](const vulkan_format_info& a, const vulkan_format_info& b) constexpr -> bool { return (int)a.format < (int)b.format; });
      //std::sort(raw_list.begin(), raw_list.end(), [](const vulkan_format_info& a, const vulkan_format_info& b) constexpr -> bool { return (int)a.format < (int)b.format; });
      return raw_list;
   }();

   constexpr const vulkan_format_info* vulkan_format_info_for(VkFormat f) {
      for (const auto& item : all_vulkan_formats) {
         if (item.format == f)
            return &item;
         if (item.format > f)
            return nullptr;
      }
      return nullptr;
   }
}
