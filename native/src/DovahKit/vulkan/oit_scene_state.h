#pragma once
#include "_vulkan.h"
#include "buffer.h"
#include "image.h"

namespace vulkanDK {
   struct oit_scene_state {
      struct {
         //
         // Buffer used to hold all pixels' linked lists of fragments. Ergo, bytecount is 
         // sizeof(element) * render_width * render_height * config::oit_layer_count.
         // 
         // If we're rendering with MSAA, we use VK_FORMAT_R32G32B32A32_UINT elements (i.e. 
         // uvec4); otherwise, VK_FORMAT_R32G32_UINT (uvec2).
         //
         buffer       data;
         VkBufferView view = VK_NULL_HANDLE;
      } a_buffer;
      concrete_image color;
      concrete_image aux;
   };
}