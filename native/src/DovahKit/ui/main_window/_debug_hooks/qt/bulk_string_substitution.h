#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::qt {
   struct bulk_string_substitution : debug_feature {
      static constexpr const char* name = "bulk_string_substitution";
      static void execute(QWidget* from);
   };
}