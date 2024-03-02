#pragma once
#include "_base.h"

namespace DovahKitDebug {
   namespace features {
      struct script_attachment_query_from_cache : debug_feature {
         static constexpr const char* name = "Check script attached w/o loading form";
         static void execute(QWidget* from);
      };
   }
}
