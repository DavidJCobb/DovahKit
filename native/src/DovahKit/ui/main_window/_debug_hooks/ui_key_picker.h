#pragma once
#include "_base.h"
// DKKeyPickerWidget

namespace DovahKitDebug {
   namespace features {
      struct ui_key_picker : debug_feature {
         static constexpr const char* name = "DKKeyPickerWidget test";
         static void execute(QWidget* from);
      };
   }
}
