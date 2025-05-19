#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_yes_no_unset_widget : debug_feature {
      static constexpr const char* name = "Test DKYesNoUnsetWidget";
      static void execute(QWidget* from);
   };
}