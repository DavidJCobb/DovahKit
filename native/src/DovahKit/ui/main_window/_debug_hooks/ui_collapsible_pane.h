#pragma once
#include "_base.h"

namespace DovahKitDebug {
   namespace features {
      struct ui_collapsible_pane : debug_feature {
         static constexpr const char* name = "DKCollapsiblePane test";
         static void execute(QWidget* from);
      };
   }
}
