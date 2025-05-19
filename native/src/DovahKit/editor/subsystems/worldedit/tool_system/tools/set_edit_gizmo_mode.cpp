#include "./set_edit_gizmo_mode.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void set_edit_gizmo_mode::request(const tool_request_cause& input, const options_union& raw_options, tool_response_tuple& all_results) {
      const auto& o = raw_options.as<options>();
      //
      response res = response(o);
      all_results.merge_member(input, res);
   }
   /*static*/ void set_edit_gizmo_mode::request_for_hold_release(const options_union& raw_options, tool_response_tuple& all_results) {
      //
      // TODO: A Hold bind should revert back to the prior gizmo mode when released.
      //
   }
}

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void set_edit_gizmo_mode::invoke(const response& params) {
      auto& worldedit_core = core::get();

      if (params.modify_gizmo) {
         auto prior = worldedit_core.get_edit_gizmo_mode();
         auto after = params.gizmo.a;
         if (prior == after && params.toggle_gizmo)
            after = params.gizmo.b;
         worldedit_core.set_edit_gizmo_mode(after);
      }
      if (params.frame.a != reference_frame::current) {
         auto prior = worldedit_core.get_edit_gizmo_frame();
         auto after = params.frame.a;
         if (prior == after && params.toggle_frame && params.frame.b != reference_frame::current)
            after = params.frame.b;
         worldedit_core.set_edit_gizmo_frame(after);
      }
   }
}