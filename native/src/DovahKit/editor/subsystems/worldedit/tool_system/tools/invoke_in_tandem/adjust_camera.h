#pragma once
#include "./_base.h"
#include "../move_camera.h"
#include "../turn_camera.h"

namespace dovahkit::subsystems::worldedit::tools::tandem {
   class adjust_camera : public invoke_in_tandem<move_camera, turn_camera> {
      public:
         static void _invoke_impl(const move_camera::response*, const turn_camera::response*);
   };
}