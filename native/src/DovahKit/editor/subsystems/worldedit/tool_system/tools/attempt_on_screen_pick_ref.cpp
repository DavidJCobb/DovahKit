#include "attempt_on_screen_pick_ref.h"
#include "../tool_response_tuple.h"
#include "../../passkeys/attempt_pick_ref.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_pick_ref::request(const tool_request_cause& input, const options_union& raw_options, tool_response_tuple& all_responses) {
      all_responses.merge_member(input, response{});
   }
   /*static*/ void attempt_on_screen_pick_ref::request_for_hold_release(const options_union&, tool_response_tuple&) {
      // No-op.
   }
}

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_pick_ref::invoke(const response&) {
      auto& worldedit_core = core::get();
      worldedit_core.attempt_pick_ref({});
   }
}