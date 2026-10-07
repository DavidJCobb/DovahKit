#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::crash_tests {
   struct multiple_failing_threads : debug_feature {
      static constexpr const char* name = "Uncaught exceptions on multiple threads";
      static void execute(QWidget* from);
   };
}