#include "turn_camera.h"
#include "_options.h"
#include "../InputResult.h"
#include "../../vulkan/data/DKVulkanCameraUpdate.h"

namespace DK3D::tools {
   void turn_camera::invoke(const InputResult& input, const opaque_option_union& raw_options, DKVulkanCameraUpdate& camera_update) const {
      if (!input.active())
         return;
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      //
      auto& cut = camera_update.turn;
      cut.speed = glm::radians(90.0F);
      cut.scale_by_delta = true;
      switch (input.type) {
         using _ = control_type;
         case _::button:
            cut.yaw   = o->magnitudes.yaw;
            cut.pitch = o->magnitudes.pitch;
            if (input.press_type != button_press_type::while_down) {
               camera_update.turn.scale_by_delta = false;
               //
               // If the user wants to just turn the camera in increments when a key is tapped, 
               // then we need to disable scaling by the time delta, but we also need to modify 
               // the turn speed to match the degree values they entered.
               //
               double speed = sqrt((cut.yaw * cut.yaw) + (cut.pitch * cut.pitch));
               cut.speed = glm::radians(speed);
            }
            break;
         case _::scalar:
         case _::vector:
            cut.yaw   = 0;
            cut.pitch = 0;
            {
               float x = input.x;
               float y = input.y;
               if (o->non_button.x_sign == Sign::Negative)
                  x = -x;
               if (o->non_button.y_sign == Sign::Negative)
                  y = -y;
               //
               switch (o->non_button.input_x) {
                  using _ = CameraTurnAxis;
                  case _::Pitch:
                     cut.pitch = x;
                     break;
                  case _::Yaw:
                     cut.yaw = x;
                     break;
               }
               switch (o->non_button.input_y) {
                  using _ = CameraTurnAxis;
                  case _::Pitch:
                     cut.pitch = y;
                     break;
                  case _::Yaw:
                     cut.yaw = y;
                     break;
               }
            }
            break;
      }
      //
      if (cut.scale_by_delta) {
         double speed = sqrt((cut.yaw * cut.yaw) + (cut.pitch * cut.pitch));
         speed = std::clamp(speed, 0.0, 1.0);
         cut.speed *= speed;
      }
   }
}