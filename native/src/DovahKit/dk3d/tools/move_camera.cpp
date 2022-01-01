#include "move_camera.h"
#include "_options.h"
#include "_results.h"
#include "../InputResult.h"
#include "../../vulkan/data/DKVulkanCameraUpdate.h"

namespace {
   void _apply(DK3D::tools::move_camera::results& out, float input, DK3D::axis3D axis, DK3D::sign sign) {
      using namespace DK3D;
      //
      if (sign == sign::negative)
         input = -input;
      switch (axis) {
         case axis3D::x:
            out.x = input;
            break;
         case axis3D::y:
            out.y = input;
            break;
         case axis3D::z:
            out.z = input;
            break;
      }
   }
}

namespace DK3D::tools {
   void move_camera::results::scale(double delta_seconds) {
      this->x *= delta_seconds;
      this->y *= delta_seconds;
      this->z *= delta_seconds;
   }
   void move_camera::results::merge(const results& from) {
      this->x += from.x;
      this->y += from.y;
      this->z += from.z;
   }

   void move_camera::invoke(const InputResult& input, const opaque_option_union& raw_options, combined_tool_results& all_results) const {
      if (!input.active())
         return;
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      //
      // TODO: Handle ReferenceFrames here, or provide an option for them in DKVulkanCameraUpdate. 
      // The former approach requires access to camera state from here; the latter approach does 
      // not, but would require us to reset the movement vector if the update state already has a 
      // different reference frame (e.g. if there are multiple inconsistent move_camera keybinds). 
      // 
      // Currently, we always treat movement as camera-relative (comments on DKVulkanCameraUpdate 
      // saying it's world-relative are currently wrong).
      //
      results res = {
         .x = o->magnitudes.x,
         .y = o->magnitudes.y,
         .z = o->magnitudes.z,
      };
      switch (input.type) {
         using _ = control_type;
         case _::button:
            break;
         case _::scalar:
         case _::vector:
            _apply(res, input.x, o->non_button.input_x, o->non_button.x_sign);
            _apply(res, input.y, o->non_button.input_y, o->non_button.y_sign);
            break;
      }
      all_results.merge_member(res);
   }
}