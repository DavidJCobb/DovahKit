#include "orbit_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

// for invoke:
#include "helpers/math/rotation/unit_conversion.h"
#include "editor/ini/main.h"
#include "../../core.h"
//
namespace {
   namespace worldedit_ini_settings {
      using namespace dovahkit::ini::main::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void orbit_camera::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;
      res.target = o.target;

      auto& vec = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      vec = o.magnitudes;
      if (o.range.has_value()) {
         if (!input.has_range)
            return;
         o.range.value().scale(vec, input);
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void orbit_camera::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }

   /*static*/ void orbit_camera::invoke(const response& params) {
      auto& worldedit_core = core::get();
      
      glm::vec3 turn = params.held.to_struct<glm::vec3>();
      turn.z *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondX.get_current_value<double>();
      turn.x *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondY.get_current_value<double>();
      //
      turn += params.instant.to_struct<glm::vec3>();

      worldedit_core.orbit_camera(
         params.target,
         turn
      );
   }
}