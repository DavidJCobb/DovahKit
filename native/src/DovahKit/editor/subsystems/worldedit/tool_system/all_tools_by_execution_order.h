#pragma once
#include "helpers/class_array.h"

namespace dovahkit::subsystems::worldedit::tools {
   // Forward-declarations only, here.
   using all_tools_by_execution_order = cobb::class_array<
      class modify_camera_speed_flags,
      class move_camera,
         // see below
      class turn_camera,
         //
         // BUG: Currently, we invoke `move_camera` and `turn_camera` in a single step: we 
         //      have a single "camera update" struct and they're both used, if present, 
         //      to populate it. How can we replicate this behavior if we're invoking tools 
         //      one by one?
         // 
         //      We'd need to define a wrapper of some kind...
         //
      class attempt_on_screen_selection,
      class debug_dump_landscape_details,
      class debug_dump_raycast,
      class debug_print,
      class set_edit_gizmo_mode//,
   >;
}