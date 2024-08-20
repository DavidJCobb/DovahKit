#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_status_bar_segment : debug_feature {
      static constexpr const char* name = "Test DKStatusBarSegment";
      static void execute(QWidget* from);
   };
}