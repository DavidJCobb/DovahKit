#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct idles_datastore : debug_feature {
      static constexpr const char* name = "Idles datastore";
      static void execute(QWidget* from);
   };
}