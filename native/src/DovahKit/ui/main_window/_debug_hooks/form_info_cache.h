#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct form_info_cache : debug_feature {
      static constexpr const char* name = "Force-instantiate form info cache";
      static void execute(QWidget* from);
   };
}