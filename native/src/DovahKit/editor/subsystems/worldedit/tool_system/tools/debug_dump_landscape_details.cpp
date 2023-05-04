#include "./debug_dump_landscape_details.h"
#include "../options_union.h"
#include "../tool_results_tuple.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void debug_dump_landscape_details::invoke(const tool_invocation_cause& input, const opaque_options_union& raw_options, tool_results_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      results res = {
         .position = o.position,
      };
      if (input.has_button) {
         if (!input.button.down_state_changed_this_frame) {
            return;
         }
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void debug_dump_landscape_details::invoke_for_hold_release(const opaque_options_union& raw_options, tool_results_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      results res = {
         .position = o.position,
      };
      all_results.merge_member(res);
   }
}