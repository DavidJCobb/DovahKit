#pragma once
#include <string_view>
#include "helpers/string/strlen.h"
#include "helpers/string/string_to_integer.h"
#include "./vulkan_formats.h"

namespace vulkanDK::data {

   //
   // unfinished
   //

   consteval vulkan_format_info vulkan_format_info_from_format_name(VkFormat fmt, const char* name) {
      vulkan_format_info out = {};
      out.format = fmt;

      constexpr const char* prefix = "VK_FORMAT_";
      constexpr const char  delim = '_';

      constexpr auto _to_next_chunk = [](std::string_view& v) {
         auto i = v.find(delim);
         if (i == -1) {
            v = {};
            return;
         }
         v = v.substr(i + 1);
      };
      constexpr auto _skip_chunks = [](std::string_view& v, size_t count) {
         for (size_t i = 0; i < count; ++i)
            _to_next_chunk(v);
      };
      constexpr auto _current_chunk = [](std::string_view& v) -> std::string_view {
         auto i = v.find(delim);
         if (i == -1)
            return v;
         return v.substr(0, i);
      };

      auto view = std::string_view(name);
      if (!view.starts_with(prefix))
         throw;
      view.remove_prefix(cobb::strlen(prefix));

      [&out, &view](){  // Check compression format
         size_t i = view.find(delim);
         
         std::string_view chunk;
         if (i == -1) {
            chunk = view;
         } else {
            chunk = view.substr(0, i);
         }
         
         if (chunk.starts_with("BC")) {
            out.compression_type = vulkan_format_compression_type::bc;
            if (chunk.size() < 3)
               throw;
            int bc_version = chunk[2] - '0';
            switch (bc_version) {
               case 1:
                  out.texel_block_size = 8;
                  break;
               case 2:
               case 3:
               case 4:
               case 5:
               case 6:
               case 7:
                  out.texel_block_size = 16;
                  break;
               default:
                  throw; // unhandled/unrecognized BC type
            }
            return;
         }
         if (chunk.starts_with("ETC2")) {
            out.compression_type = vulkan_format_compression_type::etc2;
            return;
         }
         if (chunk.starts_with("EAC")) {
            out.compression_type = vulkan_format_compression_type::eac;
            return;
         }
         if (chunk.starts_with("ASTC")) {
            out.compression_type = vulkan_format_compression_type::astc;

            _to_next_chunk(chunk);
            auto block_size_chunk = _current_chunk(chunk);

            size_t i = block_size_chunk.find('x');
            if (i == -1)
               throw; // next chunk should be of the form WxH e.g. 3x5

            auto view_w = block_size_chunk.substr(0, i);
            auto view_h = block_size_chunk.substr(i + 1);
            bool ok_w = true;
            bool ok_h = true;
            size_t w = cobb::string_to_integer<size_t>(view_w.data(), view_w.size(), &ok_w);
            size_t h = cobb::string_to_integer<size_t>(view_h.data(), view_h.size(), &ok_h);
            if (!ok_w || !ok_h)
               throw;

            out.compression_info.astc.block_size.w = w;
            out.compression_info.astc.block_size.h = h;
            out.compression_info.astc.block_size.d = 1;

            return;
         }

         out.compression_type = vulkan_format_compression_type::none;
      }();

      return out;
   }
}