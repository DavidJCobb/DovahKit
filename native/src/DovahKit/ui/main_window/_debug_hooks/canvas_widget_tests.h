#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct debug_canvas_widget : debug_feature {
      static constexpr const char* name = "Debug CanvasWidget";
      static void execute(QWidget* from);
   };
}