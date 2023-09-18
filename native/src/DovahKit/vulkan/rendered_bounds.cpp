#include "rendered_bounds.h"

namespace vulkanDK {
   void rendered_bounds::mark_for_delete() {
      base::_mark_for_delete<rendered_bounds>();
   }
   void rendered_bounds::reset() {
      base::_reset<rendered_bounds>();
   }

   void rendered_bounds::set_size(const glm::vec3& min, const glm::vec3& max) {
      this->_min = min;
      this->_max = max;
      //
      glm::vec3 local_center = (max + min) / 2.0F;
      //
      this->frame_drawing_data.pivot_offset  = local_center;
      this->frame_drawing_data.transform     = this->_pivot_transform;
      this->frame_drawing_data.transform[3] += glm::vec4(local_center, 0.0F);
      //
      glm::vec3 size = (max - min) / 2.0F;
      this->frame_drawing_data.transform[0] *= size.x;
      this->frame_drawing_data.transform[1] *= size.y;
      this->frame_drawing_data.transform[2] *= size.z;
      //
      this->on_frame_drawing_data_changed();
   }
   void rendered_bounds::set_size_and_transform(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform) {
      this->_min = min;
      this->_max = max;
      this->_pivot_transform = pivot_transform;
      //
      glm::vec3 local_center = (max + min) / 2.0F;
      //
      this->frame_drawing_data.pivot_offset  = local_center;
      this->frame_drawing_data.transform     = pivot_transform;
      this->frame_drawing_data.transform[3] += glm::vec4(local_center, 0.0F);
      //
      glm::vec3 size = (max - min) / 2.0F;
      this->frame_drawing_data.transform[0] *= size.x;
      this->frame_drawing_data.transform[1] *= size.y;
      this->frame_drawing_data.transform[2] *= size.z;
      //
      this->on_frame_drawing_data_changed();
   }
   void rendered_bounds::set_transform(const glm::mat4& pivot_transform) {
      this->_pivot_transform = pivot_transform;
      //
      glm::vec3 local_center = (this->_max + this->_min) / 2.0F;
      //
      this->frame_drawing_data.pivot_offset = local_center;
      this->frame_drawing_data.transform = pivot_transform;
      this->frame_drawing_data.transform[3] += glm::vec4(local_center, 0.0F);
      //
      glm::vec3 size = (this->_max - this->_min) / 2.0F;
      this->frame_drawing_data.transform[0] *= size.x;
      this->frame_drawing_data.transform[1] *= size.y;
      this->frame_drawing_data.transform[2] *= size.z;
      //
      this->on_frame_drawing_data_changed();
   }
}