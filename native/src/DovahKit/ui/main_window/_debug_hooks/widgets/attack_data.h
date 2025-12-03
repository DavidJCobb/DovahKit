#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct attack_data : debug_feature {
      static constexpr const char* name = "DKAttackDataWidget";
      static void execute(QWidget* from);
   };
}