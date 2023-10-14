#include "turn_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "editor/ini/main.h"

// headers for invoke
#include "helpers/math/rotation/unit_conversion.h"
#include "vulkan/data/camera_coordinate_change.h"
#include "../../core.h"
//
namespace {
   namespace worldedit_ini_settings {
      using namespace dovahkit::ini::main::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void turn_camera::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;
      auto& vec = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;

      vec = o.magnitudes;
      if (o.range.has_value()) {
         if (!input.has_range)
            return;
         o.range.value().scale(vec, input);

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

   /*static*/ void turn_camera::invoke(const response& params) {
      auto& worldedit_core = core::get();

      //
      // The "turn" params include two movement vectors: the "instant" vector and the "held" 
      // vector. The former vector represents sudden jumps triggered by instantaneous inputs 
      // e.g. the immediate press or release of a button. The latter vector represents speeds 
      // per second for movements triggered by sustained inputs, like holding a button down.
      // 
      // The "held" vector should be scaled by any relevant look sensitivity prefs, whereas the 
      // "instant" vector should not. Neither vector should be scaled by the frame delta: the 
      // "held" vector will already have been scaled within Worldinput, so multiplying in any 
      // speed-per-second values is all that's needed.
      //
      vulkanDK::data::camera_coordinate_change update;

      update.turn = params.held.to_struct<glm::vec3>();
      update.turn.z *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondX.get_current_value<double>();
      update.turn.x *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondY.get_current_value<double>();
      //
      update.turn += params.instant.to_struct<glm::vec3>();

      worldedit_core.adjust_camera(update);
   }
}