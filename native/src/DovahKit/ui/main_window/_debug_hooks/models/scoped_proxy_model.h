#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::models {
   struct scoped_proxy_model : debug_feature {
      static constexpr const char* name = "DKScopedProxyModel";
      static void execute(QWidget* from);
   };
}