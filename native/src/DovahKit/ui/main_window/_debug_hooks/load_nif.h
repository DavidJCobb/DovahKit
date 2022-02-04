#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct load_nif : debug_feature {
      static constexpr const char* name = "Test NIF loader";
      static void execute(QWidget* from);
   };
}