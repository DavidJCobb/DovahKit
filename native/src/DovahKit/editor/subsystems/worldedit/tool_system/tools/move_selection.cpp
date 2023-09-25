#include "move_selection.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }

   void _apply(cobb::vector3<float>& out, float input, worldedit::axis3D axis, worldedit::sign sign) {
      using namespace dovahkit::subsystems::worldedit;
      //
      if (sign == sign::negative)
         input = -input;
      switch (axis) {
         case axis3D::x:
            out.x *= input;
            break;
         case axis3D::y:
            out.y *= input;
            break;
         case axis3D::z:
            out.z *= input;
            break;
      }
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_selection::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      uint8_t constraint_mask = 0;
      constraint_mask |= (o.constraints.x ? 1 : 0);
      constraint_mask |= (o.constraints.y ? 1 : 0) << 1;
      constraint_mask |= (o.constraints.z ? 1 : 0) << 2;
      if (constraint_mask == 0) {
         //
         // We're not allowing movement along *any* axis... so we're not allowing movement. 
         // Exit without giving any response.
         //
  //       return; // broken; no idea why; whole thing needs revision anyway
      }

      auto constraint_frame = o.constraints.frame;
      if (constraint_frame == reference_frame::current) {
         constraint_frame = o.frame;
      }

      response res;
      auto& dst = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      if (o.follow_pointer) {
         switch (o.frame) {
            case reference_frame::camera: dst.follow_pointer.camera = true; break;
            case reference_frame::local:  dst.follow_pointer.local  = true; break;
            case reference_frame::world:  dst.follow_pointer.world  = true; break;
         }

         auto& dst_c = dst.follow_pointer.constraints;
         switch (constraint_frame) {
            case reference_frame::camera: dst_c.camera = constraint_mask; break;
            case reference_frame::local:  dst_c.local  = constraint_mask; break;
            case reference_frame::world:  dst_c.world  = constraint_mask; break;
         }
      } else {
         cobb::vector3<float> vec = o.magnitudes;
         if (input.has_range) {
            _apply(vec, input.range.x, o.range.x.axis, o.range.x.sign);
            _apply(vec, input.range.y, o.range.y.axis, o.range.y.sign);
         }
         switch (o.frame) {
            case reference_frame::camera: dst.camera = vec; break;
            case reference_frame::local:  dst.local = vec;  break;
            case reference_frame::world:  dst.world = vec;  break;
         }

         auto& dst_c = dst.constraints;
         switch (constraint_frame) {
            case reference_frame::camera: dst_c.camera = constraint_mask; break;
            case reference_frame::local:  dst_c.local  = constraint_mask; break;
            case reference_frame::world:  dst_c.world  = constraint_mask; break;
         }
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void move_selection::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}

#include "../../core.h"
#include "editor/ini/main.h"
#include "vulkan/data/camera_coordinate_change.h"
namespace {
   namespace worldedit_ini_settings {
      using namespace dovahkit::ini::main::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_selection::invoke(const response& params) {
      auto& worldedit_core = core::get();

      auto held_move = params.held.world;
      {
         held_move *= worldedit_ini_settings::fCameraSpeedNormal.get_current_value<double>();
         //
         if (worldedit_core.get_camera_speed_flag(camera_speed_flag::boost))
            held_move *= worldedit_ini_settings::fCameraSpeedMultBoost.get_current_value<double>();
         if (worldedit_core.get_camera_speed_flag(camera_speed_flag::precision))
            held_move *= worldedit_ini_settings::fCameraSpeedMultPrecision.get_current_value<double>();
      }

      auto adjust = core::coordinate_adjustment{
         .frame = reference_frame::world,
         .pos   = held_move + params.instant.world,
         .rot   = {}
      };
      bool success = worldedit_core.try_adjust_selection_coordinates(adjust);
      if (success) {
         if (false) { // TODO: "also move camera" option
            vulkanDK::data::camera_coordinate_change update;
            //
            // TODO: I don't want to copy and paste all the "move camra" code from the `adjust_camera` 
            //       invocation-in-tandem. We should factor that out into a function that takes the 
            //       "held" movement and the "instant" movement and applies movement speeds as needed.
            //
         }
      }
   }
}