#include "rendered_bounds.h"

namespace vulkanDK {
   void rendered_bounds::_on_shader_parameter_change() {
      if (this->pending_delete())
         return;
      this->handled_frames.set_all_out_of_date();
   }
   
   void rendered_bounds::set_shader_params(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform) {
      glm::vec3 local_center = (max + min) / 2.0F;
      //
      this->shader_params.pivot_offset  = local_center;
      this->shader_params.transform     = pivot_transform;
      this->shader_params.transform[3] += glm::vec4(local_center, 0.0F);
      //
      glm::vec3 size = (max - min) / 2.0F;
      this->shader_params.transform[0] *= size.x;
      this->shader_params.transform[1] *= size.y;
      this->shader_params.transform[2] *= size.z;
      //
      this->_on_shader_parameter_change();
   }
   void rendered_bounds::set_shader_params(const shader_parameters& in) {
      this->shader_params = in;
      this->_on_shader_parameter_change();
   }
}