#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include "helpers/enum_flags.h"
#include "../_vulkan.h"

namespace vulkanDK::dds {
   class dxgi_format_type {
      public:
         enum class component {
            none,
            r,
            g,
            b,
            a,
            depth,
            stencil,
            misc, // for things like shared exponents
         };

         enum class detail_flag {
            block_compressed,
            srgb,
         };
         using detail_mask = cobb::enum_flags<detail_flag, 8>;

         enum class value_type : uint8_t {
            unknown,
            typeless,
            unused, // bits are reserved rather than merely not being present
            floating_point,
            floating_point_unsigned,
            floating_point_shared_exponent,
            int_unsigned,
            int_unsigned_normalized,
            int_signed,
            int_signed_normalized,
            fixed_point_bias_2_8,
            //
            _unspecified = std::numeric_limits<uint8_t>::max(),
         };

         struct color {
            component  which    = component::none;
            //
            uint8_t    bitcount = 0;
            value_type type     = value_type::unused;
         };

      public:
         std::array<color, 4> components = {};
         detail_mask detail;
         struct {
            VkFormat format  = VkFormat::VK_FORMAT_UNDEFINED;
            uint32_t min_api = VK_MAKE_API_VERSION(0, 1, 0, 0); // 1.0
         } vulkan;

      public:
         struct params {
            size_t     component_bitcount = size_t(-1); // if all are the same
            std::array<size_t, 4> component_bitcounts = { size_t(-1), size_t(-1), size_t(-1), size_t(-1) };
            std::array<component, 4> component_order = {};
            value_type component_type = value_type::_unspecified;
            //
            // You can specify the above fields as shortcuts, or directly specify "real" fields:
            //
            decltype(dxgi_format_type::components) components;
            decltype(dxgi_format_type::detail)     detail;
            decltype(dxgi_format_type::vulkan)     vulkan;
         };
         static constexpr std::array<component, 4> order_rgb   = { component::r, component::g, component::b, component::none };
         static constexpr std::array<component, 4> order_rgba  = { component::r, component::g, component::b, component::a };
         static constexpr std::array<component, 4> order_bgr   = { component::b, component::g, component::r, component::none };
         static constexpr std::array<component, 4> order_bgra  = { component::b, component::g, component::r, component::a };
         //
         static constexpr std::array<component, 4> order_r     = { component::r, component::none, component::none, component::none };
         static constexpr std::array<component, 4> order_rg    = { component::r, component::g, component::none, component::none };
         static constexpr std::array<component, 4> order_depth = { component::depth, component::none, component::none, component::none };

         constexpr dxgi_format_type() {}
         constexpr dxgi_format_type(const params& c) : 
            components(c.components),
            detail(c.detail),
            vulkan(c.vulkan)
         {
            if (c.component_order[0] != component::none) {
               auto& src = c.component_order;
               auto& dst = this->components;
               for (size_t i = 0; i < src.size(); ++i) {
                  dst[i].which = src[i];
                  if (src[i] != component::none) {
                     if (c.component_bitcount != size_t(-1))
                        dst[i].bitcount = c.component_bitcount;
                     else if (c.component_bitcounts[i] != size_t(-1))
                        dst[i].bitcount = c.component_bitcounts[i];
                     if (c.component_type != value_type::_unspecified)
                        dst[i].type = c.component_type;
                  }
               }
            } else {
               if (c.component_bitcount != size_t(-1)) {
                  for (auto& comp : this->components)
                     comp.bitcount = c.component_bitcount;
               } else {
                  auto& src = c.component_bitcounts;
                  auto& dst = this->components;
                  for (size_t i = 0; i < src.size(); ++i)
                     if (src[i] != size_t(-1))
                        dst[i].bitcount = src[i];
               }
               if (c.component_type != value_type::_unspecified)
                  for (auto& comp : this->components)
                     comp.type = c.component_type;
            }
         }
   };
   constexpr auto dxgi_formats = std::array<dxgi_format_type, 133>{
      dxgi_format_type{ // DXGI_FORMAT_UNKNOWN
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32A32_TYPELESS
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32A32_FLOAT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32A32_UINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R32G32B32A32_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32A32_SINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R32G32B32A32_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32_TYPELESS
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgb,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32_FLOAT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgb,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R32G32B32_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32_UINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgb,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R32G32B32_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32B32_SINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rgb,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R32G32B32_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16B16A16_TYPELESS
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16B16A16_FLOAT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16B16A16_UNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_R16G16B16A16_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16B16A16_UINT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R16G16B16A16_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16B16A16_SNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_signed_normalized,
         .vulkan = {
            .format = VK_FORMAT_R16G16B16A16_SNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16B16A16_SINT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R16G16B16A16_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32_TYPELESS
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32_FLOAT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R32G32_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32_UINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R32G32_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G32_SINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R32G32_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32G8X24_TYPELESS
         .components = {{
            {
               .which    = dxgi_format_type::component::r,
               .bitcount = 32,
               .type     = dxgi_format_type::value_type::typeless,
            },
            {
               .which    = dxgi_format_type::component::g,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::typeless,
            },
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::typeless,
            },
         }},
      },
      dxgi_format_type::params{ // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
         .components = {{
            {
               .which    = dxgi_format_type::component::depth,
               .bitcount = 32,
               .type     = dxgi_format_type::value_type::floating_point,
            },
            {
               .which    = dxgi_format_type::component::stencil,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::int_unsigned,
            },
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::int_unsigned,
            },
         }},
         .vulkan = {
            .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_D32_FLOAT_S8X24_TYPELESS
         .components = {{
            {
               .which    = dxgi_format_type::component::depth,
               .bitcount = 32,
               .type     = dxgi_format_type::value_type::floating_point,
            },
            {
               .which    = dxgi_format_type::component::stencil,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::typeless,
            },
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::typeless,
            },
         }},
         .vulkan = {
            .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
         .components = {{
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 32,
               .type     = dxgi_format_type::value_type::typeless,
            },
            {
               .which    = dxgi_format_type::component::g,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::int_unsigned,
            },
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::int_unsigned,
            },
         }},
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R10G10B10A2_TYPELESS
         .component_bitcounts = { 10, 10, 10, 2 },
         .component_order     = dxgi_format_type::order_rgba,
         .component_type      = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R10G10B10A2_UNORM
         .component_bitcounts = { 10, 10, 10, 2 },
         .component_order     = dxgi_format_type::order_rgba,
         .component_type      = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R10G10B10A2_UINT
         .component_bitcounts = { 10, 10, 10, 2 },
         .component_order     = dxgi_format_type::order_rgba,
         .component_type      = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_A2R10G10B10_UINT_PACK32,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R11G11B10_FLOAT
         .component_bitcounts = { 11, 11, 10, 0 },
         .component_order     = dxgi_format_type::order_rgb,
         .component_type      = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8B8A8_TYPELESS
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8B8A8_UNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail             = dxgi_format_type::detail_flag::srgb,
         .vulkan = {
            .format = VK_FORMAT_R8G8B8A8_SRGB,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8B8A8_UINT
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R8G8B8A8_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8B8A8_SNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_signed_normalized,
         .vulkan = {
            .format = VK_FORMAT_R8G8B8A8_SNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8B8A8_SINT
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rgba,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R8G8B8A8_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16_TYPELESS
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16_FLOAT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R16G16_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16_UNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_R16G16_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16_UINT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R16G16_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16_SNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_signed_normalized,
         .vulkan = {
            .format = VK_FORMAT_R16G16_SNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16G16_SINT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R16G16_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32_TYPELESS
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_D32_FLOAT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_depth,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_D32_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32_FLOAT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R32_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32_UINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R32_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R32_SINT
         .component_bitcount = 32,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R32_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R24G8_TYPELESS
         .components = {{
            {
               .which    = dxgi_format_type::component::r,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::typeless,
            },
            {
               .which    = dxgi_format_type::component::g,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::typeless,
            },
         }},
      },
      dxgi_format_type::params{ // DXGI_FORMAT_D24_UNORM_S8_UINT
         .components = {{
            {
               .which    = dxgi_format_type::component::depth,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::int_unsigned_normalized,
            },
            {
               .which    = dxgi_format_type::component::stencil,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::int_unsigned,
            },
         }},
         .vulkan = {
            .format = VK_FORMAT_D24_UNORM_S8_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R24_UNORM_X8_TYPELESS
         .components = {{
            {
               .which    = dxgi_format_type::component::r,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::int_unsigned_normalized,
            },
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::typeless,
            },
         }},
      },
      dxgi_format_type::params{ // DXGI_FORMAT_X24_TYPELESS_G8_UINT
         .components = {{
            {
               .which    = dxgi_format_type::component::none,
               .bitcount = 24,
               .type     = dxgi_format_type::value_type::typeless,
            },
            {
               .which    = dxgi_format_type::component::g,
               .bitcount = 8,
               .type     = dxgi_format_type::value_type::int_unsigned,
            },
         }},
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8_TYPELESS
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8_UNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_R8G8_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8_UINT
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R8G8_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8_SNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_signed_normalized,
         .vulkan = {
            .format = VK_FORMAT_R8G8_SNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8_SINT
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_rg,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R8G8_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16_TYPELESS
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16_FLOAT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::floating_point,
         .vulkan = {
            .format = VK_FORMAT_R16_SFLOAT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_D16_UNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_depth,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_D16_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16_UNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_R16_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16_UINT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R16_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16_SNORM
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_signed_normalized,
         .vulkan = {
            .format = VK_FORMAT_R16_SNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R16_SINT
         .component_bitcount = 16,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R16_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8_TYPELESS
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8_UNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_R8_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8_UINT
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_unsigned,
         .vulkan = {
            .format = VK_FORMAT_R8_UINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8_SNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_signed_normalized,
         .vulkan = {
            .format = VK_FORMAT_R8_SNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8_SINT
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_signed,
         .vulkan = {
            .format = VK_FORMAT_R8_SINT,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_A8_UNORM
         .component_bitcount = 8,
         .component_order    = { dxgi_format_type::component::a },
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R1_UNORM
         .component_bitcount = 1,
         .component_order    = dxgi_format_type::order_r,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R9G9B9E5_SHAREDEXP
         .components = {{
            {
               .which    = dxgi_format_type::component::r,
               .bitcount = 9,
               .type     = dxgi_format_type::value_type::floating_point,
            },
            {
               .which    = dxgi_format_type::component::g,
               .bitcount = 9,
               .type     = dxgi_format_type::value_type::floating_point,
            },
            {
               .which    = dxgi_format_type::component::b,
               .bitcount = 9,
               .type     = dxgi_format_type::value_type::floating_point,
            },
            {
               .which    = dxgi_format_type::component::misc,
               .bitcount = 5,
               .type     = dxgi_format_type::value_type::floating_point_shared_exponent,
            },
         }},
         .vulkan = {
            .format = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R8G8_B8G8_UNORM
         .component_bitcount = 8,
         .component_order    = { dxgi_format_type::component::r, dxgi_format_type::component::g, dxgi_format_type::component::b, dxgi_format_type::component::g },
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format  = VK_FORMAT_G8B8G8R8_422_UNORM,
            .min_api = VK_MAKE_API_VERSION(0, 1, 1, 0),
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_G8R8_G8B8_UNORM
         .component_bitcount = 8,
         .component_order    = { dxgi_format_type::component::g, dxgi_format_type::component::r, dxgi_format_type::component::g, dxgi_format_type::component::b },
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format  = VK_FORMAT_B8G8R8G8_422_UNORM,
            .min_api = VK_MAKE_API_VERSION(0, 1, 1, 0),
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC1_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC1_UNORM
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC1_UNORM_SRGB
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_mask::from<dxgi_format_type::detail_flag::block_compressed, dxgi_format_type::detail_flag::srgb>(),
         .vulkan = {
            .format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC2_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC2_UNORM
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC2_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC2_UNORM_SRGB
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_mask::from<dxgi_format_type::detail_flag::block_compressed, dxgi_format_type::detail_flag::srgb>(),
         .vulkan = {
            .format = VK_FORMAT_BC2_SRGB_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC3_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC3_UNORM
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC3_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC4_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC4_UNORM
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC4_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC4_SNORM
         .component_type = dxgi_format_type::value_type::int_signed_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC4_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC5_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC5_UNORM
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC5_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC5_SNORM
         .component_type = dxgi_format_type::value_type::int_signed_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC5_SNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B5G6R5_UNORM
         .component_bitcounts = { 5, 6, 5 },
         .component_order     = dxgi_format_type::order_bgr,
         .component_type      = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_B5G6R5_UNORM_PACK16,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B5G5R5A1_UNORM
         .component_bitcounts = { 5, 5, 5, 1 },
         .component_order     = dxgi_format_type::order_bgra,
         .component_type      = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B8G8R8A8_UNORM
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_bgra,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_B8G8R8A8_UNORM,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B8G8R8X8_UNORM
         .component_bitcount = 8,
         .component_order    = { dxgi_format_type::component::b, dxgi_format_type::component::g, dxgi_format_type::component::r, dxgi_format_type::component::none },
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
         .components = {{
            {
               .which    = dxgi_format_type::component::r,
               .bitcount = 10,
               .type     = dxgi_format_type::value_type::fixed_point_bias_2_8,
            },
            {
               .which    = dxgi_format_type::component::g,
               .bitcount = 10,
               .type     = dxgi_format_type::value_type::fixed_point_bias_2_8,
            },
            {
               .which    = dxgi_format_type::component::b,
               .bitcount = 10,
               .type     = dxgi_format_type::value_type::fixed_point_bias_2_8,
            },
            {
               .which    = dxgi_format_type::component::a,
               .bitcount = 2,
               .type     = dxgi_format_type::value_type::int_unsigned_normalized,
            },
         }},
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B8G8R8A8_TYPELESS
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_bgra,
         .component_type     = dxgi_format_type::value_type::typeless
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_bgra,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail             = dxgi_format_type::detail_flag::srgb,
         .vulkan = {
            .format = VK_FORMAT_B8G8R8A8_SRGB,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B8G8R8X8_TYPELESS
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_bgr,
         .component_type     = dxgi_format_type::value_type::typeless,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
         .component_bitcount = 8,
         .component_order    = dxgi_format_type::order_bgr,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail             = dxgi_format_type::detail_flag::srgb,
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC6H_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC6H_UF16
         .component_type = dxgi_format_type::value_type::floating_point_unsigned,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC6H_UFLOAT_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC6H_SF16
         .component_type = dxgi_format_type::value_type::floating_point,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC6H_SFLOAT_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC7_TYPELESS
         .component_type = dxgi_format_type::value_type::typeless,
         .detail         = dxgi_format_type::detail_flag::block_compressed
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC7_UNORM
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_flag::block_compressed,
         .vulkan = {
            .format = VK_FORMAT_BC7_UNORM_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_BC7_UNORM_SRGB
         .component_type = dxgi_format_type::value_type::int_unsigned_normalized,
         .detail         = dxgi_format_type::detail_mask::from<dxgi_format_type::detail_flag::block_compressed, dxgi_format_type::detail_flag::srgb>(),
         .vulkan = {
            .format = VK_FORMAT_BC7_SRGB_BLOCK,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_AYUV
      },
      dxgi_format_type::params{ // DXGI_FORMAT_Y410
      },
      dxgi_format_type::params{ // DXGI_FORMAT_Y416
      },
      dxgi_format_type::params{ // DXGI_FORMAT_NV12
      },
      dxgi_format_type::params{ // DXGI_FORMAT_P010
      },
      dxgi_format_type::params{ // DXGI_FORMAT_P016
      },
      dxgi_format_type::params{ // DXGI_FORMAT_420_OPAQUE
      },
      dxgi_format_type::params{ // DXGI_FORMAT_YUY2
      },
      dxgi_format_type::params{ // DXGI_FORMAT_Y210
      },
      dxgi_format_type::params{ // DXGI_FORMAT_Y216
      },
      dxgi_format_type::params{ // DXGI_FORMAT_NV11
      },
      dxgi_format_type::params{ // DXGI_FORMAT_AI44
      },
      dxgi_format_type::params{ // DXGI_FORMAT_IA44
      },
      dxgi_format_type::params{ // DXGI_FORMAT_P8
      },
      dxgi_format_type::params{ // DXGI_FORMAT_A8P8
      },
      dxgi_format_type::params{ // DXGI_FORMAT_B4G4R4A4_UNORM
         .component_bitcount = 4,
         .component_type     = dxgi_format_type::value_type::int_unsigned_normalized,
         .vulkan = {
            .format = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
         },
      },
      dxgi_format_type::params{ // DXGI_FORMAT_P208
      },
      dxgi_format_type::params{ // DXGI_FORMAT_V208
      },
      dxgi_format_type::params{ // DXGI_FORMAT_V408
      },
   };

   using dxgi_format = uint32_t;
}