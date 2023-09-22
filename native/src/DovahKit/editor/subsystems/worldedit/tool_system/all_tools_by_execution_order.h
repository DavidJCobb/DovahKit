#pragma once
#include "helpers/class_array.h"

#include "./tools/_all.h"
#include "./tools/invoke_in_tandem/_base.h"
#include "./tools/invoke_in_tandem/adjust_camera.h"

namespace dovahkit::subsystems::worldedit::tools {
   namespace tandem {
      //
      // There are cases where multiple editing functions are separated into different 
      // tools, but under the hood, it's more efficient to handle them together. As an 
      // example, "move camera" and "turn camera" are separate actions with separate 
      // parameters and effects; however, under the hood, we pass both camera changes 
      // to the Vulkan renderer because it has to recompute the camera's transformation 
      // matrix after either one, and we don't want to recompute twice per frame.
      //
      // In these cases, subclass `tandem::invoke_in_tandem` and have your subclass 
      // define a static `invoke` function that takes as arguments the responses for 
      // each tool that should be invoked in tandem. Then, list that subclass below, 
      // and don't list the wrapped tools.
      //
   }

   using all_tools_by_execution_order = cobb::class_array<
      class modify_camera_speed_flags,
      class tandem::adjust_camera,
      class attempt_on_screen_selection,
      class debug_dump_landscape_details,
      class debug_dump_raycast,
      class debug_print,
      class set_edit_gizmo_mode
   >;
}