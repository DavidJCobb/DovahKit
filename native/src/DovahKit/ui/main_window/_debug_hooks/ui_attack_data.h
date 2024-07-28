#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_attack_data : debug_feature {
      static constexpr const char* name = "Test DKAttackDataWidget";
      static void execute(QWidget* from);
   };
}