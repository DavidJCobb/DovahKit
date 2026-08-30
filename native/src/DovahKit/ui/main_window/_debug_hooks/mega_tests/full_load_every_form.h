#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::mega_tests {
   struct full_load_every_form : debug_feature {
      static constexpr const char* name = "Test loading every form";
      static void execute(QWidget* from);
   };
}