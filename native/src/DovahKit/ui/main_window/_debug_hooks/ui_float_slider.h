#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_float_slider : debug_feature {
      static constexpr const char* name = "Test DKFloatSlider";
      static void execute(QWidget* from);
   };
}