#pragma once
#include "_base.h"

namespace DovahKitDebug {
   namespace features {
      struct ui_bound_script : debug_feature {
         static constexpr const char* name = "DKPapyrusBoundScriptListPane test";
         static void execute(QWidget* from);
      };
   }
}
