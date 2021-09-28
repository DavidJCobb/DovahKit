#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct debug_target_form : debug_feature {
      static constexpr const char* name = "Debugbreak on form";
      static void execute(QWidget* from);
   };
}