#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct key_picker : debug_feature {
      static constexpr const char* name = "DKKeyPickerWidget";
      static void execute(QWidget* from);
   };
}
