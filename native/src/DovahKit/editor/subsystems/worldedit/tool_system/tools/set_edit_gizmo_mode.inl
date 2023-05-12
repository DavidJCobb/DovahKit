#pragma once
#include "./set_edit_gizmo_mode.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void set_edit_gizmo_mode::options::read(options_serialization_version version, cobb::streams::bitreader& stream) {
      stream.read(
         frame.a,
         frame.b,
         gizmo.a,
         gizmo.b,
         toggle_frame,
         toggle_gizmo,
         modify_gizmo
      );
   }
   constexpr void set_edit_gizmo_mode::options::write(cobb::streams::bitwriter& stream) const {
      stream.write(
         frame.a,
         frame.b,
         gizmo.a,
         gizmo.b,
         toggle_frame,
         toggle_gizmo,
         modify_gizmo
      );
   }
}

