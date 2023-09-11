#include "turn_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "editor/ini/main.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void turn_camera::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;
      auto& vec = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;

      vec = { o.magnitudes.pitch, 0, o.magnitudes.yaw };
      if (input.has_range) {
         float mod_yaw   = 0;
         float mod_pitch = 0;
         switch (o.range.x.axis) {
            case camera_turn_axis::yaw:   mod_yaw   += (o.range.x.sign == sign::negative ? -1.0 : 1.0) * input.range.x; break;
            case camera_turn_axis::pitch: mod_pitch += (o.range.x.sign == sign::negative ? -1.0 : 1.0) * input.range.x; break;
         }
         switch (o.range.y.axis) {
            case camera_turn_axis::yaw:   mod_yaw   += (o.range.y.sign == sign::negative ? -1.0 : 1.0) * input.range.y; break;
            case camera_turn_axis::pitch: mod_pitch += (o.range.y.sign == sign::negative ? -1.0 : 1.0) * input.range.y; break;
         }
         vec.z *= mod_yaw;
         vec.x *= mod_pitch;

         if (dovahkit::ini::main::worldedit::bInvertLookX.get_current_value<bool>()) {
            vec.z *= -1;
         }
         if (dovahkit::ini::main::worldedit::bInvertLookY.get_current_value<bool>()) {
            vec.x *= -1;
         }
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void turn_camera::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}