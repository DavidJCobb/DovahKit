#include "./debug_dump_raycast.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "dovah/form_stub.h"

namespace {
   void _print_raycast(const dovahkit::subsystems::worldedit::raycast_result& raycast) {
      using namespace dovahkit::subsystems::worldedit;

      qDebug("Raycast");
      if (raycast.hit_position.has_value()) {
         auto& hp = raycast.hit_position.value();
         qDebug(" - Hit position: (%f, %f, %f)", hp.x, hp.y, hp.z);
         qDebug(" - View position: (%f, %f)", raycast.view_position.x(), raycast.view_position.y());
         qDebug(" - Target info:");
         if (raycast.target_info.edit_gizmo.mode != gizmo_mode::none) {
            const char* axis = "?";
            const char* type = "?";
            switch (raycast.target_info.edit_gizmo.axis) {
               case axis3D::x: axis = "x"; break;
               case axis3D::y: axis = "y"; break;
               case axis3D::z: axis = "z"; break;
            }
            switch (raycast.target_info.edit_gizmo.mode) {
               case gizmo_mode::translate: type = "translate"; break;
               case gizmo_mode::rotate:    type = "rotate";    break;
               case gizmo_mode::scale:     type = "scale";     break;
            }
            qDebug("    - Edit gizmo (%s) axis %s", type, axis);
         }
         if (raycast.target_info.form) {
            qDebug("    - [FORM:%08X]%s", raycast.target_info.form->formID, raycast.target_info.form->editorID.c_str());
         }
         if (raycast.target_info.is_selected) {
            qDebug("    - Target is selected");
         }
      } else {
         qDebug(" - No hit.");
      }
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void debug_dump_raycast::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      if (!input.button.down_state_changed_this_frame) {
         return;
      }
      if (!input.raycast.has_value()) {
         return;
      }

      _print_raycast(input.raycast.value());
   }
   /*static*/ void debug_dump_raycast::request_for_hold_release(const opaque_options_union& raw_options, tool_response_tuple& all_results) {
   }
}