#include "move_selection.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_selection::request(const tool_request_cause& input, const options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      if (o.locked_axes.no_movement_allowed())
         return;

      response res;
      auto& dst = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      {
         cobb::vector3<float> vec = o.magnitudes;
         if (o.range.has_value()) {
            if (!input.has_range)
               return;
            o.range.value().scale(vec, input);
         }
         if (o.frame != reference_frame::world) {
            auto& worldedit_core = core::get();

            auto frame_mat = worldedit_core.get_frame_rotation_matrix(o.frame);
            vec = frame_mat * vec.to_struct<glm::vec3>();
         }
         if (o.locked_axes.any_locked()) {
            if (o.locked_axes.frame == reference_frame::world) {
               if (o.locked_axes.x)
                  vec.x = 0;
               if (o.locked_axes.y)
                  vec.y = 0;
               if (o.locked_axes.z)
                  vec.z = 0;
            } else {
               auto& worldedit_core = core::get();

               glm::vec3 lock_mult = {
                  o.locked_axes.x ? 0 : 1,
                  o.locked_axes.y ? 0 : 1,
                  o.locked_axes.z ? 0 : 1
               };

               auto frame_mat = worldedit_core.get_frame_rotation_matrix(o.locked_axes.frame);
               auto frame_inv = glm::inverse(frame_mat);
               auto vec_local = vec.to_struct<glm::vec3>() * frame_mat;
               if (o.locked_axes.x)
                  vec_local.x = 0;
               if (o.locked_axes.y)
                  vec_local.y = 0;
               if (o.locked_axes.z)
                  vec_local.z = 0;
               vec = vec_local * frame_inv;
            }
         }
         dst.magnitude = vec;
         
         if (o.also_move_camera)
            dst.camera.magnitude = vec;
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void move_selection::request_for_hold_release(const options_union&, tool_response_tuple&) {
      // No-op.
   }
}

#include "vulkan/data/camera_coordinate_change.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_selection::invoke(const response& params) {
      auto& worldedit_core = core::get();

      auto speed = worldedit_core.get_camera_move_speed();

      auto held_move = params.held.magnitude * speed;

      auto adjust = core::coordinate_adjustment{
         .rotate = {
            .euler = {},
            .frame = reference_frame::world,
         },
         .translate = held_move + params.instant.magnitude,
      };
      bool success = worldedit_core.try_adjust_selection_coordinates(adjust);
      if (success) {
         auto cam_move = params.held.camera.magnitude * speed;
         cam_move += params.instant.camera.magnitude;

         if (cam_move.x || cam_move.y || cam_move.z) {
            worldedit_core.translate_camera(cam_move, reference_frame::world);
         }
      }
   }
}