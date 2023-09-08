#include "./debug_dump_landscape_details.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void debug_dump_landscape_details::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      response res = {};
      if (input.has_button) {
         if (!input.button.down_state_changed_this_frame) {
            return;
         }
      }
      if (input.raycast.has_value()) {
         const auto& raycast = input.raycast.value();
         if (raycast.hit_position.has_value()) {
            res.hit_position = raycast.hit_position.value();
            res.target       = raycast.target_info.form;
         }
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void debug_dump_landscape_details::request_for_hold_release(const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      response res = {};
      all_results.merge_member(res);
   }
}