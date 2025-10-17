#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct idle_animations_model : debug_feature {
      static constexpr const char* name = "IdleAnimationFormsModel treeview";
      static void execute(QWidget* from);
   };
}