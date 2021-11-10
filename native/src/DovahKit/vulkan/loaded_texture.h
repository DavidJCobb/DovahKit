#pragma once
#include <QString>
#include "_vulkan.h"
#include "helpers/frames_in_flight.h"
#include "image.h"

namespace vulkanDK {
   struct loaded_texture {
      concrete_image content;
      //
      uint32_t w = 0;
      uint32_t h = 0;
      QString  path;
      //
      frames_in_flight_mask frame_dirty_flags; // for normal textures: frames that need descriptors resynchronized. for pending-delete textures: frames that may still be using the texture in their descriptors.
      bool     pending_delete = false; // unhook the texture from frames' descriptors; delete it when it's fully unhooked
      uint32_t refcount       = 0;

      void reset();
   };
}
