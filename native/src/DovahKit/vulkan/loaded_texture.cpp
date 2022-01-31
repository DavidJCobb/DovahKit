#include "loaded_texture.h"

namespace vulkanDK {
   void loaded_texture::mark_for_delete() {
      this->life_state = scene_frame_item_state::pending_delete;
      this->handled_frames.set_all_out_of_date();
   }
   void loaded_texture::reset() {
      this->content.teardown();
      this->w = 0;
      this->h = 0;
      this->path.clear();
      this->life_state     = scene_frame_item_state::empty;
      this->handled_frames = frame_dirty_state();
   }
}