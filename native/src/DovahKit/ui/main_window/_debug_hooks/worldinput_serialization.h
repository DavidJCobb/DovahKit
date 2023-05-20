#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct worldinput_serialization : debug_feature {
      static constexpr const char* name = "Worldinput2 serialization";
      static void execute(QWidget* from);
   };
}