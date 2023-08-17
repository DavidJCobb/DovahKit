#include "turn_camera.h"
#include "../options_union.h"
#include "../tool_results_tuple.h"

#include "editor/ini/main.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void turn_camera::invoke(const tool_invocation_cause& input, const opaque_options_union& raw_options, tool_results_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      // TODO: Handle ReferenceFrames here, or provide an option for them in DKVulkanCameraUpdate. 
      // The former approach requires access to camera state from here; the latter approach does 
      // not, but would require us to reset the movement vector if the update state already has a 
      // different reference frame (e.g. if there are multiple inconsistent turn_camera keybinds). 
      // 
      // Currently, we always treat movement as camera-relative (comments on DKVulkanCameraUpdate 
      // saying it's world-relative are currently wrong).
      //
      results res = {
         .yaw   = o.magnitudes.yaw,
         .pitch = o.magnitudes.pitch,
      };
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
         res.yaw   *= mod_yaw;
         res.pitch *= mod_pitch;
         
         if (dovahkit::ini::main::worldedit::bInvertLookX.get_current_value<bool>()) {
            res.yaw *= -1;
         }
         if (dovahkit::ini::main::worldedit::bInvertLookY.get_current_value<bool>()) {
            res.pitch *= -1;
         }
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void turn_camera::invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&) {
      // No-op.
   }
}