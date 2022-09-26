#include "rendered_light.h"

namespace vulkanDK {
   rendered_light::~rendered_light() {
      this->reset();
   }
   void rendered_light::mark_for_delete() {
      base::_mark_for_delete<rendered_light>();
      //
      // For lights pending delete, we want to set the color and radius to 0. 
      // This is so that shaders don't need to branch to check if the light 
      // is active; they can just blindly run all lights.
      //
      auto& sp = this->frame_drawing_data;
      sp.color  = { 0, 0, 0 };
      sp.fade   = 0;
      sp.radius = 0;
      sp.type   = rendered_light::light_type::omni; // set type, too, so we don't try to render shadows
   }
   void rendered_light::reset() {
      base::_reset<rendered_light>();
   }

   void rendered_light::set_transform(const glm::mat4& in) {
      this->frame_drawing_data.transform     = in;
      this->frame_drawing_data.transform_inv = glm::inverse(in);
      this->on_frame_drawing_data_changed();
   }
}