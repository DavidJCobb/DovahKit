#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct debug_form_picker : debug_feature {
      static constexpr const char* name = "DKFormPicker";
      static void execute(QWidget* from);
   };
}