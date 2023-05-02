#include "move_camera.h"
#include "../options_union.h"
#include "../tool_results_tuple.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }

   void _apply(worldedit::tools::move_camera::results& out, float input, worldedit::axis3D axis, worldedit::sign sign) {
      using namespace dovahkit::subsystems::worldedit;
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

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_camera::invoke(const tool_invocation_cause& input, const opaque_options_union& raw_options, tool_results_tuple& all_results) {
      const options& o = raw_options.as<options>();
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
         .x = o.magnitudes.x,
         .y = o.magnitudes.y,
         .z = o.magnitudes.z,
      };
      if (!input.is_button()) {
         _apply(res, input.range.x, o.range.x.axis, o.range.x.sign);
         _apply(res, input.range.y, o.range.y.axis, o.range.y.sign);
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void move_camera::invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&) {
      // No-op.
   }
}