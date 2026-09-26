#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::crash_tests {
   struct access_violation : debug_feature {
      static constexpr const char* name = "Access violation";
      static void execute(QWidget* from);
   };
}