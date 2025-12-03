#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct form_inventory : debug_feature {
      static constexpr const char* name = "DKFormInventoryWidget";
      static void execute(QWidget* from);
   };
}