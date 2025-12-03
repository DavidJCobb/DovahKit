#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct yes_no_unset_widget : debug_feature {
      static constexpr const char* name = "DKYesNoUnsetWidget";
      static void execute(QWidget* from);
   };
}