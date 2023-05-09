#pragma once
#include "editor/subsystems/worldedit/raycast_result.h"
#include "./inputs/button.h"
#include "./chrono.h"

namespace dovahkit::subsystems::worldinput2 {
   using raycast_result = worldedit::raycast_result;

   struct raycast_result_per_key : public raycast_result {
      inputs::button button;
      timestamp_t    when;
   };
}