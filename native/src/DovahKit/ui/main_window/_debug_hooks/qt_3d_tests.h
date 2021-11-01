#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct qt_3d_tests : debug_feature {
      static constexpr const char* name = "Initial 3D tests";
      static void execute(QWidget* from);
   };
}