#pragma once
#include "helpers/class_array.h"
#include "./tool_options_attempt_on_screen_selection.h"
#include "./tool_options_debug_log.h"
#include "./tool_options_debug_placeholder.h"
#include "./tool_options_modify_camera_speed_flags.h"
#include "./tool_options_move_camera.h"
#include "./tool_options_turn_camera.h"

namespace DK3DToolOptions {
   using all = cobb::class_array<
      AttemptOnScreenSelection,
      DebugLog,
      DebugPlaceholder,
      ModifyCameraSpeedFlags,
      MoveCamera,
      TurnCamera
   >;

   static_assert(
      []() -> bool {
         bool valid = true;
         all::for_each([&valid]<typename Current>() {
            if constexpr (!requires { typename Current::tool_type; }) {
               valid = false;
            }
         });
         return valid;
      }(),
      "Each widget must specify the type of the tool it configures, via a `tool_type` member type (using declaration)."
   );
}