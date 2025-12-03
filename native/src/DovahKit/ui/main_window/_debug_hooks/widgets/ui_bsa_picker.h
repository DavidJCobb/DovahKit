#pragma once
#include "../_base.h"
#include <QLabel>

namespace DovahKitDebug::features::widgets {
   struct ui_bsa_picker : debug_feature {
      static constexpr const char* name = "BSA-packed filepicker test";
      static void execute(QWidget* from);
   };
}
