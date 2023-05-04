#pragma once
#include "helpers/class_array.h"

namespace dovahkit::subsystems::worldedit::tools {
   // Forward-declarations only, here.
   using all_tools = cobb::class_array<
      class attempt_on_screen_selection,
      class debug_dump_landscape_details,
      class debug_print,
      class modify_camera_speed_flags,
      class move_camera,
      class turn_camera//,
   >;
}