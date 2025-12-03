#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct status_bar_segment : debug_feature {
      static constexpr const char* name = "DKStatusBarSegment";
      static void execute(QWidget* from);
   };
}