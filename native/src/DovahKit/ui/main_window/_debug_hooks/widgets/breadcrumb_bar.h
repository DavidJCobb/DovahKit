#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct breadcrumb_bar : debug_feature {
      static constexpr const char* name = "DKBreadcrumbBar";
      static void execute(QWidget* from);
   };
}