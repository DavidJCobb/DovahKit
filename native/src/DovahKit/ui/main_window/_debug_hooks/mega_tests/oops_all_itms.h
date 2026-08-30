#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::mega_tests {
   struct oops_all_itms : debug_feature {
      static constexpr const char* name = "Create ITMs of all records in a specified file (backend only)";
      static void execute(QWidget* from);
   };
}