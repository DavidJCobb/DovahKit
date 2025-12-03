#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct bound_script_list_pane : debug_feature {
      static constexpr const char* name = "DKPapyrusBoundScriptListPane";
      static void execute(QWidget* from);
   };
}
