#pragma once
#include "_base.h"
#include "../../helpers/class_list.h"

#include "debug_log.h"
#include "move_camera.h"
#include "turn_camera.h"

namespace DK3D {
   using all_editor_functions = cobb::class_list<
      editor_functions::debug_log,
      editor_functions::move_camera,
      editor_functions::turn_camera//,
   >;
}