#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct dk3d_debug_rebind : debug_feature {
      static constexpr const char* name = "DK3DInputHandler: open temp rebind dialog";
      static void execute(QWidget* from);
   };
}