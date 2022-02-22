#include "rendered_light.h"

namespace vulkanDK {
   rendered_light::~rendered_light() {
      this->reset();
   }

   void rendered_light::_on_shader_parameter_change() {
      if (this->pending_delete())
         return;
      this->handled_frames.set_all_out_of_date();
   }
   //
   void rendered_light::set_transform(const glm::mat4& in) {
      this->shader_params.transform = in;
      this->_on_shader_parameter_change();
   }

   void rendered_light::mark_for_delete() {
      this->life_state = scene_frame_item_state::pending_delete;
      this->handled_frames.set_all_out_of_date();
      //
      auto& sp = this->shader_params;
      sp.color  = { 0, 0, 0 };
      sp.radius = 0;
   }
   void rendered_light::reset() {
      this->handled_frames = frame_dirty_state();
      this->life_state = scene_frame_item_state::empty;
   }
}