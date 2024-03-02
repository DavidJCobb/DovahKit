#pragma once
#include "_base.h"

namespace DovahKitDebug {
   namespace features {
      struct filter_object_selection_by_scriptname : debug_feature {
         static constexpr const char* name = "UI: Test filtering object selection by scriptname";
         static void execute(QWidget* from);
      };
   }
}
