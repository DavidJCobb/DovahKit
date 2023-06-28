#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct worldinput2_control_scheme_rebuild : debug_feature {
      static constexpr const char* name = "Worldinput2 control scheme node reimp";
      static void execute(QWidget* from);
   };
}