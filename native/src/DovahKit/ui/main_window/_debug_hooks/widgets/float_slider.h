#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct float_slider : debug_feature {
      static constexpr const char* name = "DKFloatSlider";
      static void execute(QWidget* from);
   };
}