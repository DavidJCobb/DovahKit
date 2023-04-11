#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct worldinput2 : debug_feature {
      static constexpr const char* name = "Worldinput2 tests";
      static void execute(QWidget* from);
   };
}