#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct compiled_papyrus_script_tests : debug_feature {
      static constexpr const char* name = "Compiled Papyrus script tests";
      static void execute(QWidget* from);
   };
}