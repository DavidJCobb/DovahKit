#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct worldinput2_control_scheme_ui : debug_feature {
      static constexpr const char* name = "Worldinput2 control scheme editor";
      static void execute(QWidget* from);
   };
}