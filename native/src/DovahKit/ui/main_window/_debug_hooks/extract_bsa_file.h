#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct extract_bsa_file : debug_feature {
      static constexpr const char* name = "Extract single file from BSA";
      static void execute(QWidget* from);
   };
}