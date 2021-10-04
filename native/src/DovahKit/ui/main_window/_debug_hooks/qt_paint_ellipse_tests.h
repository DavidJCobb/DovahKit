#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct qt_paint_ellipse_tests : debug_feature {
      static constexpr const char* name = "QPainter(Path) ellipse tests";
      static void execute(QWidget* from);
   };
}