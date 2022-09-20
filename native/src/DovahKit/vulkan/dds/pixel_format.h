#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <type_traits>

namespace cobb {
   class generic_reader_ex;
}

namespace vulkanDK::dds {
   struct pixel_format {
      static constexpr size_t serialized_size = 32;

      struct flag {
         enum type : uint32_t {
            has_alpha        = 0x00000001, // texture contains valid alpha data; if not set, channel_masks.a is undefined
            alpha_only       = 0x00000002, // older DDS files: alpha-only uncompressed data
            has_four_cc      = 0x00000004, // texture contains compresesd RGB data; four_cc is defined
            uncompressed_rgb = 0x00000040, // texture contains uncompresesd RGB data; RGB bitcount and the RGB masks are defined
            has_yuv_colors   = 0x00000200, // older DDS files: texture contains uncompressed YUV colors (swap YUV for RGB; bitcount and masks are defined)
            has_luminance    = 0x00020000, // older DDS files: red is luminance channel; RGB bitcount is luminance channel bitcount; red mask is defined; can use has_alpha for two channels
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      struct format_code { // big-endian
         enum type : uint32_t {
            dxt1 = 'DXT1',
            dxt2 = 'DXT2',
            dxt3 = 'DXT3',
            dxt4 = 'DXT4',
            dxt5 = 'DXT5',
            extended_header = 'DX10',
         };
      };
      using four_cc_t = std::underlying_type_t<format_code::type>;

      uint32_t  size    = serialized_size; // size of this structure
      flags_t   flags   = 0;
      four_cc_t four_cc = 0;
      uint32_t  rgb_bitcount = 0; // bits per pixel
      union {
         std::array<uint32_t, 4> list = {};
         struct {
            uint32_t r;
            uint32_t g;
            uint32_t b;
            uint32_t a;
         };
      } channel_masks;

      constexpr bool has_four_cc() const noexcept { return 0 != (this->flags & flag::has_four_cc); };
      constexpr bool has_rgb_bitcount() const noexcept { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_luminance | flag::has_yuv_colors)); };
      constexpr bool use_extended_header() const noexcept { return this->has_four_cc() && (this->four_cc == format_code::extended_header); };

      constexpr bool has_channel_r() const noexcept { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors | flag::has_luminance)); }
      constexpr bool has_channel_g() const noexcept { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors)); }
      constexpr bool has_channel_b() const noexcept { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors)); }
      constexpr bool has_channel_a() const noexcept { return 0 != (this->flags & (flag::has_alpha | flag::alpha_only)); }

      constexpr bool is_uncompressed() const noexcept { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors | flag::has_luminance | flag::alpha_only)); }

      static constexpr size_t max_color_channel_count = std::tuple_size_v<decltype(decltype(channel_masks)::list)>;

      constexpr bool channels_overlap() const noexcept {
         for (size_t i = 0; i - 1 < max_color_channel_count; ++i) {
            auto mask_a = this->channel_masks.list[i];
            for (size_t j = i + 1; j < max_color_channel_count; ++j) {
               auto mask_b = this->channel_masks.list[i];
               auto diff = mask_a ^ mask_b;
               if (diff != (mask_a | mask_b)) {
                  return true;
               }
            }
         }
         return false;
      }

      constexpr bool channels_partially_overlap() const noexcept {
         for (size_t i = 0; i - 1 < max_color_channel_count; ++i) {
            auto mask_a = this->channel_masks.list[i];
            for (size_t j = i + 1; j < max_color_channel_count; ++j) {
               auto mask_b = this->channel_masks.list[i];
               auto diff   = mask_a ^ mask_b;
               if (diff != 0 && diff != (mask_a | mask_b)) { // (diff == 0) means total overlap; (diff == (mask_a | mask_b)) means no overlap
                  return true;
               }
            }
         }
         return false;
      }

      // If all present channels have the same bitcount, return that bitcount; else, zero.
      constexpr uint8_t uniform_channel_bitcount() const noexcept {
         uint8_t bitcount = 0;
         for (size_t i = 0; i < max_color_channel_count; ++i) {
            auto mask = this->channel_masks.list[i];
            if (!mask)
               continue;
            auto channel_bc = std::bit_width(mask >> std::bit_width(mask & -(int32_t)mask)) + 1;
            if (bitcount == 0) {
               bitcount = channel_bc;
            } else {
               if (bitcount != channel_bc)
                  return 0;
            }
         }
         return bitcount;
      }

      // Returns the order in which R, G, B, and A occur. Only present channels are written; if fewer than 4 channels are 
      // used, the array will have the value -1 appear at the end to fill absent chnanels.
      //
      // Return value entries are: channel index (RGBA = 0123; -1 for absent); and LSB position.
      std::array<std::pair<int8_t, int>, 4> channel_sequence() const noexcept;

      void read(cobb::generic_reader_ex&); // can throw
   };
}