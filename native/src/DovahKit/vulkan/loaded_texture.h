#pragma once
#include <QString>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "image.h"
#include "scene_frame_item.h"

namespace vulkanDK {
   struct loaded_texture {
      public:
         struct flag { // flags applied on the CPU, not within shaders
            enum type : uint32_t {
               is_default_land_texture = 0x00000001,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         flags_t flags = 0;
         //
         owned_image_and_view content;
         //
         uint32_t w = 0;
         uint32_t h = 0;
         QString  path;
         //
         frame_dirty_state      handled_frames; // for normal textures: frames that have had descriptors resynchronized. for pending-delete textures: frames that have unhooked this texture from their descriptors.
         scene_frame_item_state life_state = scene_frame_item_state::empty;
         uint32_t refcount = 0;

         inline bool active() const noexcept { return this->life_state == scene_frame_item_state::active; }
         inline bool empty() const noexcept { return this->life_state == scene_frame_item_state::empty; }
         inline bool pending_delete() const noexcept { return this->life_state == scene_frame_item_state::pending_delete; }

         void mark_for_delete();
         void reset();

         bool persist_for_life_of_renderer() const;
   };
}
