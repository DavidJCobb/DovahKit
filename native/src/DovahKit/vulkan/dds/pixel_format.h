#pragma once
#include <array>
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

      inline bool has_four_cc() const { return 0 != (this->flags & flag::has_four_cc); };
      inline bool has_rgb_bitcount() const { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_luminance | flag::has_yuv_colors)); };
      inline bool use_extended_header() const { return this->has_four_cc() && (this->four_cc == format_code::extended_header); };

      inline bool has_channel_r() const { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors | flag::has_luminance)); }
      inline bool has_channel_g() const { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors)); }
      inline bool has_channel_b() const { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors)); }
      inline bool has_channel_a() const { return 0 != (this->flags & (flag::has_alpha | flag::alpha_only)); }

      inline bool is_uncompressed() const { return 0 != (this->flags & (flag::uncompressed_rgb | flag::has_yuv_colors | flag::has_luminance | flag::alpha_only)); }

      void read(cobb::generic_reader_ex&); // can throw
   };
}