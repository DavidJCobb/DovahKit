#include "turn_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "editor/ini/main.h"

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
}