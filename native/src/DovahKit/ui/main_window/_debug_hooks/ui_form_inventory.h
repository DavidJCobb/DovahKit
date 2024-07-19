#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_form_inventory : debug_feature {
      static constexpr const char* name = "Test DKFormInventory";
      static void execute(QWidget* from);
   };
}