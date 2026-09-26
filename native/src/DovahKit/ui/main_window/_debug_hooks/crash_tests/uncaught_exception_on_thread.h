#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::crash_tests {
   struct uncaught_exception_on_thread : debug_feature {
      static constexpr const char* name = "Uncaught exception on another thread";
      static void execute(QWidget* from);
   };
}