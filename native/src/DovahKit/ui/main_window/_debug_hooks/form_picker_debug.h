#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct debug_form_picker : debug_feature {
      static constexpr const char* name = "Debug Formpicker";
      static void execute(QWidget* from);
   };
}