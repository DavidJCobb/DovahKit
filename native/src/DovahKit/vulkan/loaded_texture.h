#pragma once
#include <QString>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "image.h"

namespace vulkanDK {
   struct loaded_texture {
      concrete_image content;
      //
      uint32_t w = 0;
      uint32_t h = 0;
      QString  path;
      //
      frame_dirty_state handled_frames; // for normal textures: frames that have had descriptors resynchronized. for pending-delete textures: frames that have unhooked this texture from their descriptors.
      bool     pending_delete = false; // unhook the texture from frames' descriptors; delete it when it's fully unhooked
      uint32_t refcount       = 0;

      void mark_for_delete();
      void reset();
   };
}
