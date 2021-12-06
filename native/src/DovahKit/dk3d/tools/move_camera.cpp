#include "move_camera.h"
#include "_options.h"
#include "../InputResult.h"
#include "../../vulkan/data/DKVulkanCameraUpdate.h"

namespace {
   void _apply(glm::vec3& out, float input, DK3D::Axis3D axis, DK3D::Sign sign) {
      using namespace DK3D;
      //
      if (sign == Sign::Negative)
         input = -input;
      switch (axis) {
         case Axis3D::X:
            out.x = input;
            break;
         case Axis3D::Y:
            out.y = input;
            break;
         case Axis3D::Z:
            out.z = input;
            break;
      }
   }
}

namespace DK3D::tools {
   void move_camera::invoke(const InputResult& input, const opaque_option_union& raw_options, DKVulkanCameraUpdate& camera_update) {
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
      glm::vec3 move = { o->magnitudes.x, o->magnitudes.y, o->magnitudes.z };
      camera_update.move.scale_by_delta = true;
      switch (input.type) {
         using _ = InputResult::Type;
         case _::Boolean:
            if (input.bool_mod != BooleanInputMod::While) {
               camera_update.move.scale_by_delta = false;
            }
            break;
         case _::Scalar:
         case _::Vector:
            move = glm::vec3();
            _apply(move, input.x, o->non_button.input_x, o->non_button.x_sign);
            _apply(move, input.y, o->non_button.input_y, o->non_button.y_sign);
            break;
      }
      camera_update.move.direction = move;
   }
}