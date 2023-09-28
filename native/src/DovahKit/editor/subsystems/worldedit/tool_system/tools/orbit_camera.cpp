#include "orbit_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }

   void _apply(cobb::vector3<float>& out, float input, worldedit::camera_turn_axis axis, worldedit::sign sign) {
      using namespace dovahkit::subsystems::worldedit;
      //
      if (sign == sign::negative)
         input = -input;
      switch (axis) {
         case camera_turn_axis::pitch:
            out.x *= input;
            break;
         case camera_turn_axis::yaw:
            out.z *= input;
            break;
      }
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void orbit_camera::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;
      res.target = o.target;

      auto& vec = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      vec.x = o.magnitudes.pitch;
      vec.z = o.magnitudes.yaw;
      if (input.has_range) {
         _apply(vec, input.range.x, o.range.x.axis, o.range.x.sign);
         _apply(vec, input.range.y, o.range.y.axis, o.range.y.sign);
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void orbit_camera::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}