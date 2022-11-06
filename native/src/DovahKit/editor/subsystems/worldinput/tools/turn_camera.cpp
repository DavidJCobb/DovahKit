#include "turn_camera.h"
#include "./_options.h"
#include "./_results.h"
#include "../input_result.h"
#include "vulkan/data/DKVulkanCameraUpdate.h"

namespace dovahkit::subsystems::worldinput::tools {
   void turn_camera::results::scale(double delta_seconds) {
      this->yaw   *= delta_seconds;
      this->pitch *= delta_seconds;
      this->roll  *= delta_seconds;
   }
   void turn_camera::results::merge(const results& from) {
      this->yaw   += from.yaw;
      this->pitch += from.pitch;
      this->roll  += from.roll;
   }

   void turn_camera::invoke(const input_result& input, const opaque_option_union& raw_options, combined_tool_results& all_results) const {
      if (!input.active())
         return;
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      //
      double  speed = glm::radians(90.0F);
      results res;
      switch (input.type) {
         using _ = control_type;
         case _::button:
            res.yaw   = o->magnitudes.yaw;
            res.pitch = o->magnitudes.pitch;
            break;
         case _::scalar:
         case _::vector:
            {
               float x = input.x;
               float y = input.y;
               if (o->non_button.x_sign == sign::negative)
                  x = -x;
               if (o->non_button.y_sign == sign::negative)
                  y = -y;
               //
               switch (o->non_button.input_x) {
                  using _ = camera_turn_axis;
                  case _::pitch:
                     res.pitch = x;
                     break;
                  case _::yaw:
                     res.yaw = x;
                     break;
               }
               switch (o->non_button.input_y) {
                  using _ = camera_turn_axis;
                  case _::pitch:
                     res.pitch = y;
                     break;
                  case _::yaw:
                     res.yaw = y;
                     break;
               }
            }
            break;
      }
      all_results.merge_member(res);
   }
}