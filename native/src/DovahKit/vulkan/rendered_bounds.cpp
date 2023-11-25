#include "rendered_bounds.h"

namespace vulkanDK {
   void rendered_bounds::mark_for_delete() {
      base::_mark_for_delete<rendered_bounds>();
   }
   void rendered_bounds::reset() {
      base::_reset<rendered_bounds>();
   }

   /*
    
      Rendered bounds are drawn as follows:

       - They don't actually contain any vertex data.

       - For the pivot, we tell Vulkan that we're passing 6 vertices, even though we're not passing 
         any. Since the vertex shader never actually trues to read vertex data, this is fine. The 
         vertex shader relies on the vertex index to know what to do.

         The pivot is rendered as three lines, one for each axis, intersecting at the center. All we 
         do is use the vertex index to determine which axis we're drawing a line for, and whether we 
         are on the positive or negative end of that line. We then compute gl_Position based on the 
         bounds' overall transform, the pivot offset, and a positive/negative offset along the given 
         axis to actually produce a line.

       - For the bounds, we use a very similar trick, except this time, our "virtual shape" is a box 
         with intrinsic dimensions spanning from corner (-1, -1, -1) to corner (1, 1, 1). The bounds' 
         overall transformation matrix serves to position this box, rotate it, and most importantly, 
         scale it to match the bounds' dimensions.

         This, then, is what the "pivot offset" is for. The transformation matrix we use must refer 
         to the box's centerpoint, but the actual pivot can be off-center. We account for this by 
         using the "pivot offset" to knock the pivot off-center within the box, and we also apply the 
         "pivot offset" when generating the transformation matrix itself in order to knock the box 
         off-center from the original "intended" position.
    
   */

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
      
      glm::vec3 local_center = (max + min) / 2.0F;

      //
      // If the transformation matrix uses uniform scaling, then we need to apply that scale 
      // factor to the bounds' internal offsets. (Non-uniform scaling is not supported, as it 
      // won't apply to our use case: REFRs.)
      //
      float scale = 1.0;
      scale = glm::length(pivot_transform[0]);
      if (scale <= 0.0001) {
         scale = glm::length(pivot_transform[1]);
         if (scale <= 0.0001) {
            scale = glm::length(pivot_transform[2]);
            if (scale <= 0.0001) {
               scale = 1.0; // fallback
            }
         }
      }
      local_center *= scale;
      
      this->frame_drawing_data.pivot_offset  = local_center;
      this->frame_drawing_data.transform     = pivot_transform;
      this->frame_drawing_data.transform[3] += glm::vec4(local_center, 0.0F);
      
      glm::vec3 size = (max - min) / 2.0F;
      this->frame_drawing_data.transform[0] *= size.x;
      this->frame_drawing_data.transform[1] *= size.y;
      this->frame_drawing_data.transform[2] *= size.z;
      //
      this->on_frame_drawing_data_changed();
   }

   void rendered_bounds::set_transform(const glm::mat4& pivot_transform) {
      this->_pivot_transform = pivot_transform;
      
      glm::vec3 local_center = (this->_max + this->_min) / 2.0F;

      //
      // If the transformation matrix uses uniform scaling, then we need to apply that scale 
      // factor to the bounds' internal offsets. (Non-uniform scaling is not supported, as it 
      // won't apply to our use case: REFRs.)
      //
      float scale = 1.0;
      scale = glm::length(pivot_transform[0]);
      if (scale <= 0.0001) {
         scale = glm::length(pivot_transform[1]);
         if (scale <= 0.0001) {
            scale = glm::length(pivot_transform[2]);
            if (scale <= 0.0001) {
               scale = 1.0; // fallback
            }
         }
      }
      local_center *= scale;
      
      this->frame_drawing_data.pivot_offset = local_center;
      this->frame_drawing_data.transform = pivot_transform;
      this->frame_drawing_data.transform[3] += glm::vec4(local_center, 0.0F);
      
      glm::vec3 size = (this->_max - this->_min) / 2.0F;
      this->frame_drawing_data.transform[0] *= size.x;
      this->frame_drawing_data.transform[1] *= size.y;
      this->frame_drawing_data.transform[2] *= size.z;
      //
      this->on_frame_drawing_data_changed();
   }
}