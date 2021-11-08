#include "rendered_mesh.h"

namespace vulkanDK {
   void rendered_mesh::_on_shader_parameter_change() {
      if (this->pending_delete)
         return;
      this->frame_dirty_flags.set_all();
   }
   //
   void rendered_mesh::set_transform(const glm::mat4& in) {
      this->shader_params.transform = in;
      this->_on_shader_parameter_change();
   }
}