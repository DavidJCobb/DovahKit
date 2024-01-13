#pragma once
#include "_base.h"

namespace DovahKitDebug {
   namespace features {
      struct ui_ref_picker : debug_feature {
         static constexpr const char* name = "DKObjectReferencePicker test";
         static void execute(QWidget* from);
      };
   }
}
