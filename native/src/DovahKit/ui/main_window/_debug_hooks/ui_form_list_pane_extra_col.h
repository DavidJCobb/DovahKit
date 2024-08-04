#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_form_list_pane_extra_col : debug_feature {
      static constexpr const char* name = "Test DKFormListPane extra columns";
      static void execute(QWidget* from);
   };
}