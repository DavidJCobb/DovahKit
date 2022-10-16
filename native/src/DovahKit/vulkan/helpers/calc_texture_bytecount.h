#pragma once
#include <array>
#include "helpers/arrays/map.h"
#include "helpers/unreachable.h"
#include "../data/vulkan_formats.h"

//
// The `vulkanDK::helpers::calc_texture_bytecount` function takes a VkFormat and 
// texture dimensions, and computes the bytecount for a texture with that size 
// (assuming no layers, mipmaps, et cetera).
// 
// This is simpler than you might expect because for efficiency's sake, texture 
// formats must support "random access:" it must be possible to look up any 
// texel (pixel in the texture data) at any coordinate, without any significant 
// slowdown. This means that the contents of a texture's data must be laid out 
// in a predictable manner even if the texture is compressed.
// 
// In practice, textures are compressed into distinct "blocks," each of which 
// has a consistent size in bytes, such that it is easy to compute which block 
// will contain a given texel and then seek to that block in the texture file. 
// Instead of compressing pixel data to the maximum possible extent, the goal 
// is to compress it sorta well enough to reduce size, while still allowing 
// fast lookups.
// 
// This in turn means that every compression algorithm will produce consistent 
// sizes given consistent image dimensions. Some algorihtms can be tuned, but 
// the tuning parameters must be represented in the VkFormat value (as can be 
// seen with the ASTC formats, wherein details about block sizing are listed 
// right in the VkFormat name).
// 
// As for uncompressed textures? It's just bits per pixel times the texture's 
// width and height.
// 
// ----------------------------------------------------------------------------
// 
// We have a large constexpr table mapping VkFormat values to information about 
// each format. However, we don't want to have to sift through the entire table 
// every time we need to query a texture size. Accordingly, we use templates to 
// generate functions that incorporate needed data via if-constexpr statements, 
// and store these functions in a list of their own.
// 
// In essence, for every VkFormat value, we are generating a single function 
// that should be wholly self-contained, running only the tests necessary to 
// compute texture data bytecounts given texture dimensions.
// 
// The base `calc_texture_bytecount` function looks up the appropriate such 
// function, and then invokes it. If you're going to be running the function 
// multiple times for the same format (e.g. for mipmapping), then you can avoid 
// redundant lookups by getting the function pointer yourself: just call the 
// `get_texture_bytecount_calc_function` function, and then pass the texture 
// dimensions to the returned function pointer (make sure it isn't null first).
//

namespace vulkanDK::helpers {
   namespace impl::_calc_texture_bytecount {
      size_t _div_ceil(size_t a, int b) {
         auto res = (a + (b - 1)) / b;
         if (res < 1)
            return 1;
         return res;
      };

      template<VkFormat Format> requires (data::vulkan_format_info_for(Format) != nullptr)
      VkDeviceSize exec(size_t w, size_t h) {
         using comp_type = data::vulkan_format_compression_type;

         constexpr const auto& info = *data::vulkan_format_info_for(Format);
         if constexpr (info.compression_type == comp_type::none) {
            return w * h * info.texel_block_size;
         } else if constexpr (info.compression_type == comp_type::astc) {
            constexpr const auto& bs = info.compression_info.astc.block_size;

            auto blocks_w = _div_ceil(w, bs.w);
            auto blocks_h = _div_ceil(h, bs.h);
            auto blocks_d = 1;
            if (bs.d) {
               blocks_d = (1 + (bs.d - 1)) / bs.d;
            }
            return blocks_w * blocks_h * blocks_d * info.texel_block_size;
         } else if constexpr (info.compression_type == comp_type::bc) {
            return _div_ceil(w, 4) * _div_ceil(h, 4) * info.texel_block_size;
         } else if constexpr (info.compression_type == comp_type::etc2 || info.compression_type == comp_type::eac) {
            return _div_ceil(w, 4) * _div_ceil(h, 4) * info.texel_block_size;
         }
         cobb::unreachable();
      }

      using exec_type = decltype(&exec<VK_FORMAT_R8G8B8_UINT>);

      struct functor_info {
         VkFormat  format;
         exec_type functor;
      };

      inline constexpr const auto functors = [](){
         constexpr const auto& src = data::all_vulkan_formats;
         return cobb::arrays::map_tp<src>([]<const auto& Array, size_t Index>() -> functor_info {
            return functor_info{
               .format  = Array[Index].format,
               .functor = &exec<Array[Index].format>,
            };
         });
      }();

      // Verify correctness of cobb::map_ct:
      static_assert(
         []() -> bool {
            if (functors.size() != data::all_vulkan_formats.size())
               return false;
            for (size_t i = 0; i < functors.size(); ++i)
               if (functors[i].format != data::all_vulkan_formats[i].format)
                  return false;
            return true;
         }()
      );
   }

   VkDeviceSize calc_texture_bytecount(VkFormat f, size_t w, size_t h) {
      for (const auto& item : impl::_calc_texture_bytecount::functors) {
         if (item.format == f)
            return item.functor(w, h);
         if (item.format > f)
            break;
      }
      return 0;
   }
   impl::_calc_texture_bytecount::exec_type get_texture_bytecount_calc_function(VkFormat f) {
      for (const auto& item : impl::_calc_texture_bytecount::functors) {
         if (item.format == f)
            return item.functor;
         if (item.format > f)
            break;
      }
      return nullptr;
   }
}