#include "move_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

// headers for invoke
#include "../../core.h"
#include "editor/ini/main.h"
//
namespace {
   namespace worldedit_ini_settings {
      using namespace dovahkit::ini::main::worldedit;
   }

   // If enabled, "held" movement vectors (see comments below) will be normalized if their length 
   // is greater than one. This prevents "strafe-running" and other speed quirks. This shouldn't 
   // be necessary for gamepad input, but would be needed for keyboard input.
   constexpr const bool normalize_super_movements = true;
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_camera::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;

      auto& vec = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      vec = o.magnitudes;
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
      all_results.merge_member(input, res);
   }
   /*static*/ void move_camera::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }

   /*static*/ void move_camera::invoke(const response& params) {
      auto& worldedit_core = core::get();

      //
      // The "move" params include two movement vectors: the "instant" vector and the "held" 
      // vector. The former vector represents sudden jumps triggered by instantaneous inputs 
      // e.g. the immediate press or release of a button. The latter vector represents speeds 
      // per second for movements triggered by sustained inputs, like holding a button down.
      // 
      // The "held" vector should be scaled by any relevant movement speed prefs, whereas the 
      // "instant" vector should not. Neither vector should be scaled by the frame delta: the 
      // "held" vector will already have been scaled within Worldinput, so multiplying in any 
      // speed-per-second values is all that's needed.
      //
      auto move = params.held.to_struct<glm::vec3>();
      if constexpr (normalize_super_movements) {
         const auto len = glm::length(move);
         if (len > 1.0)
            move /= len;
      }
      //
      // Apply movement speeds per second:
      //
      move *= worldedit_core.get_camera_move_speed();
      move += params.instant.to_struct<glm::vec3>();

      worldedit_core.translate_camera(move, reference_frame::world);
   }
}