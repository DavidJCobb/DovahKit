#include "./set_edit_gizmo_mode.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void set_edit_gizmo_mode::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const auto& o = raw_options.as<options>();
      //
      response res = response(o);
      all_results.merge_member(input, res);
   }
   /*static*/ void set_edit_gizmo_mode::request_for_hold_release(const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      //
      // TODO: A Hold bind should revert back to the prior gizmo mode when released.
      //
   }
}