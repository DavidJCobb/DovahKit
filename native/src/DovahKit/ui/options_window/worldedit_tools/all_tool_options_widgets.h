#pragma once
#include "helpers/class_array.h"

namespace dovahkit::ui::worldedit {
   namespace tools {
      class attempt_on_screen_selection;
      class debug_print;
      class modify_camera_speed_flags;
      class move_camera;
      class move_selection;
      class set_edit_gizmo_mode;
      class turn_camera;
   }

   using all_tool_options_widgets = cobb::class_array<
      tools::attempt_on_screen_selection,
      tools::debug_print,
      tools::modify_camera_speed_flags,
      tools::move_camera,
      tools::move_selection,
      tools::set_edit_gizmo_mode,
      tools::turn_camera//,
   >;
}