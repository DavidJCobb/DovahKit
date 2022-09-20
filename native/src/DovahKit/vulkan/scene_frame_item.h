#pragma once

namespace vulkanDK {
   enum class scene_frame_item_state {
      empty,
      active,
      pending_delete, // unhook the item from each frame (e.g. unhook vertex buffers from command buffers; unhook textures from descriptors); then delete it when it's fully unhooked
      pending_reload,
   };
}