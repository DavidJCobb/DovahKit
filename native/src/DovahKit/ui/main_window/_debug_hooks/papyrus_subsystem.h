#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct papyrus_subsystem : debug_feature {
      static constexpr const char* name = "Papyrus subsystem";
      static void execute(QWidget* from);
   };
}