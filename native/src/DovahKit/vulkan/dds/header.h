#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include "dxgi_format.h"
#include "pixel_format.h"
#include "../_vulkan.h"

namespace cobb {
   class generic_reader_ex;
}

namespace vulkanDK::dds {
   struct header_extension { // DDS_HEADER_DX10
      static constexpr size_t serialized_size = 20;

      enum class resource_dimension : uint32_t {
         unknown   = 0,
         buffer    = 1,
         texture1D = 2,
         texture2D = 3,
         texture3D = 4,
      };

      struct misc_flag_a {
         enum type : uint32_t {
            is_cubemap = 0x00000004,
         };
      };
      struct misc_flag_b {
         enum type : uint32_t {
            alpha_unknown       = 0x00000000, // legacy files; just assume straight alpha
            alpha_straight      = 0x00000001,
            alpha_premultiplied = 0x00000002,
            alpha_opaque        = 0x00000003, // all alpha content is opaque
            alpha_custom        = 0x00000004, // alpha is just being used as a fourth channel, and isn't meant to represent transparency
         };
      };

      dxgi_format format = 0;
      resource_dimension dimension = resource_dimension::unknown;
      uint32_t misc_flags_a = 0;
      uint32_t array_size   = 0; // for cubemap textures, this is the number of cubes
      uint32_t misc_flags_b = 0;

      void read(cobb::generic_reader_ex&);
   };

   struct header { // DDS_HEADER
      static constexpr size_t serialized_size = 124;

      struct flag {
         enum type : uint32_t {
            has_capabilities = 0x00000001,
            has_height       = 0x00000002,
            has_width        = 0x00000004,
            has_pitch        = 0x00000008,
            has_pixel_format = 0x00001000,
            has_mipmap_count = 0x00020000,
            has_linear_size  = 0x00080000,
            has_depth        = 0x00800000,
            //
            required_flags   = has_capabilities | has_height | has_width | has_pixel_format,
         };
      };
      struct capabilities_0 {
         enum type : uint32_t {
            is_complex   = 0x00000008, // should be used for mipmapped textures and cubemaps, but some exporters don't generate it...
            uses_mipmaps = 0x00400000,
            is_texture   = 0x00001000, // was meant to be mandatory, but some exporters don't generate it...
         };
      };
      struct capabilities_1 {
         enum type : uint32_t {
            is_cubemap        = 0x00000200,
            has_cubemap_x_pos = 0x00000400,
            has_cubemap_x_neg = 0x00000800,
            has_cubemap_y_pos = 0x00001000,
            has_cubemap_y_neg = 0x00002000,
            has_cubemap_z_pos = 0x00004000,
            has_cubemap_z_neg = 0x00008000,
            is_volume         = 0x00200000,
            //
            has_all_cubemap_faces = has_cubemap_x_pos | has_cubemap_x_neg | has_cubemap_y_pos | has_cubemap_y_neg | has_cubemap_z_pos | has_cubemap_z_neg, // Direct3D 9 allows partial cubemaps; Direct3D 10, 10.1,and 11 require all six faces
         };
      };

      uint32_t size  = serialized_size; // size of this structure
      uint32_t flags = flag::required_flags;
      uint32_t height;
      uint32_t width;
      union {
         uint32_t pitch;
         uint32_t linear_size = 0;
      };
      uint32_t depth;
      uint32_t mipmap_count;
      uint32_t reserved_a[11];
      pixel_format format;
      std::array<uint32_t, 4> capabilities;
      uint32_t reserved_b;
      //
      header_extension dx10_header; // only if has_extended_header() is true

      inline bool has_extended_header() const { return this->format.use_extended_header(); }

      void read(cobb::generic_reader_ex&);
      VkFormat to_vulkan_format() const;
   };
}