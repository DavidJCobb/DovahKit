#pragma once
#include <glm/glm.hpp>
#include "helpers/frame_dirty_state.h"
#include "scene_frame_item.h"

namespace vulkanDK {
   class rendered_bounds {
      public:
         struct box_vertex {
            glm::vec3 pos;
            glm::vec3 color;
         };
         struct center_vertex {
            glm::vec3 pos;
         };

         glm::mat4 transform; // use stretch/skew to set bounds
         //
         frame_dirty_state      handled_frames;
         scene_frame_item_state life_state = scene_frame_item_state::empty;

         inline bool active() const noexcept { return this->life_state == scene_frame_item_state::active; }
         inline bool empty() const noexcept { return this->life_state == scene_frame_item_state::empty; }
         inline bool pending_delete() const noexcept { return this->life_state == scene_frame_item_state::pending_delete; }

         void mark_for_delete() {
            this->life_state = scene_frame_item_state::pending_delete;
            this->handled_frames.set_all_out_of_date();
         }
         void reset() {}
   };
}