#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::qt {
   struct qpalette : debug_feature {
      static constexpr const char* name = "QPalette viewer";
      static void execute(QWidget* from);
   };
}