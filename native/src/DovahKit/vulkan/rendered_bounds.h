#pragma once
#include <glm/glm.hpp>
#include "helpers/frame_dirty_state.h"
#include "scene_frame_item.h"

namespace vulkanDK {
   class rendered_bounds {
      protected:
         void _on_shader_parameter_change();
      public:
         struct shader_parameters { // pass to the shader via a storage buffer
            alignas(16) glm::mat4 transform;    // centerpoint position, rotation, and box size
            alignas(16) glm::vec3 pivot_offset; // pivot's offset from the centerpoint
         };

         shader_parameters shader_params;
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

         void set_shader_params(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform);
         void set_shader_params(const shader_parameters&);
   };
}