#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::crash_tests {
   struct uncaught_exception : debug_feature {
      static constexpr const char* name = "Uncaught exception";
      static void execute(QWidget* from);
   };
}