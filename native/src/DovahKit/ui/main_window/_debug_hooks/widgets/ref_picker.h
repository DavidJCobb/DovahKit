#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct ref_picker : debug_feature {
      static constexpr const char* name = "DKObjectReferencePicker";
      static void execute(QWidget* from);
   };
}
