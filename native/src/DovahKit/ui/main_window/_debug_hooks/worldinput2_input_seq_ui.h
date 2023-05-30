#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct worldinput2_input_seq_ui : debug_feature {
      static constexpr const char* name = "Worldinput2 input sequence editor";
      static void execute(QWidget* from);
   };
}