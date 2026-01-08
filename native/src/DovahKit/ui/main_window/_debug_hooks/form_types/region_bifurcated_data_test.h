#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::form_types {
   struct region_bifurcated_data_test : debug_feature {
      static constexpr const char* name = "Region bifurcated data test";
      static void execute(QWidget* from);
   };
}