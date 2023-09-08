#include "attempt_on_screen_selection.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_selection::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_responses) {
      const options& o = raw_options.as<options>();
      //
      response res = {
         .operation = o.operation,
      };
      if (input.has_button) {
         if (input.button.press_type == worldinput::button_press_type::hold && !input.button.down_state_changed_this_frame) {
            res.sweep = true;
         }
      } else if (input.has_range) {
         res.sweep = true;
      }
      if (input.raycast.has_value()) {
         const auto& raycast = input.raycast.value();
         if (raycast.hit_position.has_value()) {
            res.hit_position = raycast.hit_position.value();
            res.target       = raycast.target_info.form;
         }
      }
      all_responses.merge_member(input, res);
   }
   /*static*/ void attempt_on_screen_selection::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}