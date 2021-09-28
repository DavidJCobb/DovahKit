#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct debug_target_form_papyrus : debug_feature {
      static constexpr const char* name = "Debugbreak on form Papyrus data";
      static void execute(QWidget* from);
   };
}