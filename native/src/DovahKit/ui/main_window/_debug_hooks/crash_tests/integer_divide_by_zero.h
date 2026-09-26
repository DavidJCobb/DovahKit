#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::crash_tests {
   struct integer_divide_by_zero : debug_feature {
      static constexpr const char* name = "Integer division by zero";
      static void execute(QWidget* from);
   };
}